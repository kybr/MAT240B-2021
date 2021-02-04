#include "al/core.hpp"
#include "al/util/ui/al_ControlGUI.hpp"
#include "al/util/ui/al_Parameter.hpp"
#include "al/util/ui/al_Preset.hpp"
using namespace al;

#include "synths.h"
using namespace diy;

#include <forward_list>
#include <string>
#include <unordered_set>
#include <vector>
using namespace std;

// this special container for grains offers O(1) complexity for
// getting some inactive grain (for recycling) and O(n) complexity
// for deactivating completed grains.
//
template <typename T>
class Bag {
  forward_list<T*> remove, inactive;
  unordered_set<T*> active;

 public:
  // add a grain to the container with "inactive" status.
  //
  void insert_inactive(T& t) { inactive.push_front(&t); }

  // are there any inactive grains?
  //
  bool has_inactive() { return !inactive.empty(); }

  // find the first inactive grain, make it active, and return it. you better
  // have called has_inactive() before you call this!
  //
  T& get_next_inactive() {
    T* t = inactive.front();
    active.insert(t);
    inactive.pop_front();
    return *t;
  }

  // run a given function on each active grain.
  //
  void for_each_active(function<void(T& t)> f) {
    for (auto& t : active) f(*t);
  }

  // schedule an active grain for deactivation.
  //
  void schedule_for_deactivation(T& t) { remove.push_front(&t); }

  // deactivate all grains scheduled for deactivation.
  //
  void execute_deactivation() {
    for (auto e : remove) {
      active.erase(e);
      inactive.push_front(e);
    }
    remove.clear();
  }
};

struct Granulator {
  vector<diy::Array> arrayList;

  // knows how to load a file into the granulator
  //
  void load(string fileName) {
    SearchPaths searchPaths;
    searchPaths.addSearchPath("..");

    string filePath = searchPaths.find(fileName).filepath();
    arrayList.emplace_back();
    if (arrayList.back().load(filePath)) {
      printf("Loaded %s! at %08X with size %lu\n", filePath.c_str(),
             &arrayList.back(), arrayList.back().size());
    } else {
      exit(1);
    }
  }

  // we define a Grain...
  //
  struct Grain {
    Array* source = nullptr;
    Line index;  // this is like a tape play head that scrubs through the source
    AttackDecay envelop;  // new class handles the fade in/out and amplitude
    float pan;

    float operator()() { return source->get(index()) * envelop(); }
  };

  // we store a "pool" of grains which may or may not be active at any time
  //
  vector<Grain> grain;
  Bag<Grain> bag;

  Granulator() {
    // rather than using new/delete and allocating memory on the fly, we just
    // allocate as many grains as we might need---a fixed number that we think
    // will be enough. we can find this number through trial and error. if
    // too many grains are active, we may take too long in the audio callback
    // and that will cause drop-outs and glitches.
    //
    grain.resize(1000);
    for (auto& g : grain) bag.insert_inactive(g);
  }

  // gui tweakable parameters
  //
  ParameterInt whichClip{"/clip", "", 0, "", 0, 8};
  Parameter grainDuration{"/duration", "", 0.25, "", 0.001, 1.0};
  Parameter startPosition{"/position", "", 0.25, "", 0.0, 1.0};
  Parameter peakPosition{"/envelope", "", 0.1, "", 0.0, 1.0};
  Parameter amplitudePeak{"/amplitude", "", 0.707, "", 0.0, 1.0};
  Parameter panPosition{"/pan", "", 0.5, "", 0.0, 1.0};
  Parameter playbackRate{"/playback", "", 0.0, "", -1.0, 1.0};
  Parameter birthRate{"/frequency", "", 55, "", 0, 200};

  // this oscillator governs the rate at which grains are created
  //
  Edge grainBirth;

  // this method makes a new grain out of a dead / inactive one.
  //
  void recycle(Grain& g) {
    // choose which sound clip this grain pulls from
    g.source = &arrayList[whichClip];

    // startTime and endTime are in units of sample
    float startTime = g.source->size() * startPosition;
    float endTime =
        startTime + grainDuration * SAMPLE_RATE * powf(2.0, playbackRate);

    g.index.set(startTime, endTime, grainDuration);

    // riseTime and fallTime are in units of second
    float riseTime = grainDuration * peakPosition;
    float fallTime = grainDuration - riseTime;
    g.envelop.set(riseTime, fallTime, amplitudePeak);

    g.pan = panPosition;
  }

  // make the next sample
  //
  diy::FloatPair operator()() {
    // figure out if we should generate (recycle) more grains; then do so.
    //
    grainBirth.frequency(birthRate);
    if (grainBirth())
      if (bag.has_inactive()) recycle(bag.get_next_inactive());

    // figure out which grains are active. for each active grain, get the next
    // sample; sum all these up and return that sum.
    //
    float left = 0, right = 0;
    bag.for_each_active([&](Grain& g) {
      float f = g();
      left += f * (1 - g.pan);
      right += f * g.pan;
      if (g.index.done()) bag.schedule_for_deactivation(g);
    });
    bag.execute_deactivation();

    return {left, right};
  }
};

struct MyApp : App {
  float background = 0.21;

  Granulator granulator;
  ControlGUI gui;
  PresetHandler presetHandler{"GranulatorPresets"};
  PresetServer presetServer{"0.0.0.0", 9011};

  void onCreate() override {
    // load sound files into the
    granulator.load("0.wav");
    granulator.load("1.wav");
    granulator.load("2.wav");
    granulator.load("3.wav");
    granulator.load("4.wav");
    granulator.load("5.wav");
    granulator.load("6.wav");
    granulator.load("7.wav");
    granulator.load("8.wav");

    gui.init();
    /*
    gui.addr(presetHandler,  //
             granulator.whichClip, granulator.grainDuration,
             granulator.startPosition, granulator.peakPosition,
             granulator.amplitudePeak, granulator.panPosition,
             granulator.playbackRate, granulator.birthRate);
            */
    gui << presetHandler  //
        << granulator.whichClip << granulator.grainDuration
        << granulator.startPosition << granulator.peakPosition
        << granulator.amplitudePeak << granulator.panPosition
        << granulator.playbackRate << granulator.birthRate;

    presetHandler << granulator.whichClip << granulator.grainDuration
                  << granulator.startPosition << granulator.peakPosition
                  << granulator.amplitudePeak << granulator.panPosition
                  << granulator.playbackRate << granulator.birthRate;
    presetHandler.setMorphTime(1.0);
    // presetServer << presetHandler;

    parameterServer() << granulator.whichClip << granulator.grainDuration
                      << granulator.startPosition << granulator.peakPosition
                      << granulator.amplitudePeak << granulator.panPosition
                      << granulator.playbackRate << granulator.birthRate;
    parameterServer().print();
  }

  void onAnimate(double dt) override {
    navControl().active(!gui.usingInput());
    //
  }

  void onDraw(Graphics& g) override {
    g.clear(background);
    gui.draw(g);
  }

  void onSound(AudioIOData& io) override {
    while (io()) {
      diy::FloatPair p = granulator();
      io.out(0) = p.left;
      io.out(1) = p.right;
    }
  }
};

int main() {
  MyApp app;
  app.initAudio(SAMPLE_RATE, BLOCK_SIZE, OUTPUT_CHANNELS, INPUT_CHANNELS);
  app.start();
}
