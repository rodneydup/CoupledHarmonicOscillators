#include "./Roboto-Medium.hpp"
#include "./calculations.hpp"
#include "Gamma/DFT.h"
#include "Gamma/Filter.h"
#include "Gamma/Noise.h"
#include "Gamma/Oscillator.h"
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Ray.hpp"
#include "al/sound/al_Reverb.hpp"
#include "al/ui/al_BoundingBox.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_Pickable.hpp"
using namespace al;

class CHON : public App {
 public:
  /**
   * @brief Initilialize the synth interface.
   */
  virtual void onInit() override;

  /**
   * @brief Run once on starup.
   */
  virtual void onCreate() override;

  /**
   * @brief Audio rate processing of synth.
   */
  virtual void onSound(al::AudioIOData &io) override;

  void drawAudioIO(al::AudioIO *io);

  virtual void onAnimate(double dt) override;

  /**
   * @brief Draw rate processing of synth interface.
   */
  virtual void onDraw(al::Graphics &g) override;

  virtual void onResize(int width, int height) override;

  virtual Vec3d unproject(Vec3d screenPos);

  virtual Rayd getPickRay(int screenX, int screenY);

  virtual bool onMouseMove(const Mouse &m) override;

  virtual bool onMouseDown(const al::Mouse &m) override;

  virtual bool onMouseDrag(const Mouse &m) override;

  virtual bool onMouseUp(const Mouse &m) override;

  virtual bool onKeyDown(al::Keyboard const &k) override;

  virtual bool onKeyUp(al::Keyboard const &k) override;

  virtual void chonReset();

  /** MIDI Stuff **/
  //   void initMIDI();
  //   void updateActiveMIDIParams(const al::MIDIMessage &m);

  /**
   * @brief Called everytime a MIDI message is sent.
   */
  //   virtual void onMIDIMessage(const al::MIDIMessage &m) override;

  //   virtual void onExit() override;

  Mesh mesh;           // mesh for drawing particles
  int nX = 4;          // number of particles on x axis
  int xParticles = 4;  // Parameter for x particles count
  int nY = 1;          // number of particles on y axis
  int yParticles = 1;  // Parameter for y particles count
  bool twoDimensions = false;
  std::vector<Spring *> xSprings;
  std::vector<Spring *> ySprings;
  std::vector<std::vector<Particle>> particle;  // 2D vector containing our Particles

  double springLength;          // Spacing between particles
  bool freedom[3] = {1, 0, 0};  // Degrees of freedom for particle movement
  Texture texBlur;              // blurring filter for graph
  int amCounter;                // AM incrementor
  int picked = -1;              // variable keeping track of which particle is selected
  std::mutex resetLock;
  float w = 2;
  float h = 2;

  Reverb<float> reverb;
  gam::NoiseWhite<> tick;
  bool drawGUI = 1;

  /*
  GUI variables
  */
  std::unique_ptr<ChonBundle> xSpringGUI;
  std::unique_ptr<ChonBundle> ySpringGUI;
  Parameter mAll{"Mass", "physics", 1.0f, "", 1.0f, 100.0f};  // Master mass
  Parameter b{"Damping", "physics", 0.0f, "", 0.0f, 3.0f};    // damping
  ParameterBool xFree{"X axis", "Degrees of Freedom", 1};
  ParameterBool yFree{"Y axis", "Degrees of Freedom", 0};
  ParameterBool zFree{"Z axis", "Degrees of Freedom", 0};
  ParameterBool pause{"Pause (press p)", "physics", 0};  // Pause Simulation

  ParameterBool DrawGraph{"Draw Graph", "draw", 1};               // Toggle Drawing Graph
  Parameter graphSpread{"Spread", "draw", 0.0f, "", 0.0f, 1.0f};  // Graph spread
  Parameter graphSpeed{"Speed", "draw", 10.0f, "", 1.0f, 30.0f};  // Graph draw speed
  ParameterMenu graphAxis{"Graph Axis"};  // choose which axis displacement to draw
  ParameterBool drawParticles{"Draw Particles", "draw", 1};    // Toggle drawing particles
  ParameterBool drawBoundaries{"Draw Boundaries", "draw", 0};  // Toggle drawing boundaries

  ParameterBool stereoOn{"Stereo", "Synthesis", 0};                    // Stereo Mode toggle
  ParameterBool additiveSynthOn{"Additive Synth On", "Synthesis", 0};  // Additive Synth toggle
  ParameterBool bellSynthOn{"Bell Synth On", "Synthesis", 0};          // Bell Synth toggle
  ParameterMenu bellAxis{"Axis##bell"};
  ParameterMenu bellScale{"Scale##bell"};                                   // Tuning of Bell synth
  Parameter bellRoot{"Root##bell", "Synthesis", 60.0f, "", 1.0f, 1000.0f};  // Root of bell tuning
  Parameter bellVolume{"Volume##bell", "Synthesis", 0.5f, "", 0.0f, 1.0f};  // Volume of bell synth

  Parameter additiveRoot{"Root##additive", "Synthesis", 60.0f, "", 1.0f,
                         1000.0f};  // root of additive synth
  Parameter additiveVolume{
    "Volume##additive", "Synthesis", 0.5f, "", 0.0f, 1.0f};  // Volume of bell synth

  ParameterBool fm{"FM On", "Synthesis", 0};  // FM toggle
  ParameterMenu fmAxis{"Axis##FM"};
  Parameter fmFreq{
    "Frequency##FM", "Synthesis", 1.5f, "", 0.0f, 2.0f};              // FM freq (ratio to carrier)
  Parameter fmWidth{"Width##FM", "Synthesis", 2.0f, "", 0.1f, 5.0f};  // FM Width

  ParameterBool am{"AM On", "Synthesis", 0};  // AM toggle
  ParameterMenu amAxis{"Axis##AM"};

  ParameterBool reverbOn{"Reverb On", "Synthesis", 0};               // Reverb
  Parameter reverbTail{"Tail", "Synthesis", 0.15f, "", 0.0f, 1.0f};  // Reverb decay time

  ParameterBool inputOn{"Input On", "Audio", 0};
  ParameterMenu inputMode{"Input Mode"};
  ParameterBool driveStereoSplit{"Stereo Split", "Audio", 0};
  ParameterInt driveParticleXLeft{"X##Left", "Audio", 1, 1, nX};
  ParameterInt driveParticleYLeft{"Y##Left", "Audio", 1, 1, nY};
  ParameterMenu driveAxisLeft{"Drive Axis##Left"};
  ParameterInt driveParticleXRight{"X##Right", "Audio", 1, 1, nX};
  ParameterInt driveParticleYRight{"Y##Right", "Audio", 1, 1, nY};
  ParameterMenu driveAxisRight{"Drive Axis##Right"};
  Parameter inputThreshold{"Input Threshold", "Audio", 1.0f, 0.0f, 2.0f};
  Parameter inputScale{"Input Scaling", "Audio", 1.0f, 0.1f, 2.0f};
  ParameterInt rmsSize{"RMS Samples", "Audio", 2048, 512, 4096};

  /*
  Scales
  */
  std::vector<float> majScale{1.000000,  1.125000,  1.250000,  1.333333,  1.500000,  1.666667,
                              1.875000,  2.000000,  2.250000,  2.500000,  2.666667,  3.000000,
                              3.333333,  3.750000,  4.000000,  4.500000,  5.000000,  5.333333,
                              6.000000,  6.666667,  7.500000,  8.000000,  9.000000,  10.000000,
                              10.666667, 12.000000, 13.333333, 15.000000, 16.000000, 18.000000,
                              20.000000, 21.333333, 24.000000, 26.666667, 30.000000, 32.000000};
  std::vector<float> pentScale{1.000000,  1.125000,  1.250000,  1.500000,  1.666667,  2.000000,
                               2.250000,  2.500000,  3.000000,  3.333333,  4.000000,  4.500000,
                               5.000000,  6.000000,  6.666667,  8.000000,  9.000000,  10.000000,
                               12.000000, 13.333333, 16.000000, 18.000000, 20.000000, 24.000000,
                               26.666667, 32.000000};
  std::vector<float> chromScale{
    1.000000,  1.066667,  1.125000,  1.285714,  1.250000,  1.333333,  1.406250,  1.500000,
    1.600000,  1.666667,  1.750000,  1.875000,  2.000000,  2.133333,  2.250000,  2.571429,
    2.500000,  2.666667,  2.812500,  3.000000,  3.200000,  3.333333,  3.500000,  3.750000,
    4.000000,  4.266667,  4.500000,  5.142857,  5.000000,  5.333333,  5.625000,  6.000000,
    6.400000,  6.666667,  7.000000,  7.500000,  8.000000,  8.533333,  9.000000,  10.285714,
    10.000000, 10.666667, 11.250000, 12.000000, 12.800000, 13.333333, 14.000000, 15.000000,
    16.000000, 17.066667, 18.000000, 20.571429, 20.000000, 21.333333, 22.500000, 24.000000,
    25.600000, 26.666667, 28.000000, 30.000000, 32.000000};

  std::vector<float> otSeries{
    1.000000,  2.000000,  3.000000,  4.000000,  5.000000,  6.000000,  7.000000,  8.000000,
    9.000000,  10.000000, 11.000000, 12.000000, 13.000000, 14.000000, 15.000000, 16.000000,
    17.000000, 18.000000, 19.000000, 20.000000, 21.000000, 22.000000, 23.000000, 24.000000,
    25.000000, 26.000000, 27.000000, 28.000000, 29.000000, 30.000000, 31.000000, 32.000000,
    33.000000, 34.000000, 35.000000, 36.000000, 37.000000, 38.000000, 39.000000, 40.000000,
    41.000000, 42.000000, 43.000000, 44.000000, 45.000000, 46.000000, 47.000000, 48.000000,
    49.000000, 50.000000, 51.000000, 52.000000, 53.000000, 54.000000, 55.000000, 56.000000,
    57.000000, 58.000000, 59.000000, 60.000000, 61.000000, 62.000000, 63.000000, 64.000000};
  std::vector<float> bpScale{
    1.000000,  1.080000,  1.190476,  1.285714,  1.400000,  1.530612,  1.666667,  1.800000,
    1.960000,  2.142857,  2.333333,  2.520000,  2.777778,  3.000000,  3.240000,  3.571429,
    3.857143,  4.200000,  4.591837,  5.000000,  5.400000,  5.880000,  6.428571,  7.000000,
    7.560000,  8.333333,  9.000000,  9.720000,  10.714286, 11.571429, 12.600000, 13.775510,
    15.000000, 16.200000, 17.640000, 19.285714, 21.000000, 22.680000, 25.000000, 27.000000};
  std::vector<float> *scale = &pentScale;

  /*
  OSC
  */
  int port = 16447;             // osc port
  char addr[10] = "127.0.0.1";  // ip address
  osc::Send client;             // create an osc client
  ParameterBool oscOn{"OSC On", "OSC", 0};
  ParameterBool oscX{"X disp", "OSC", 0};
  ParameterBool oscY{"Y disp", "OSC", 0};
  ParameterBool oscZ{"Z disp", "OSC", 0};
  ParameterBool oscPan{"Pan", "OSC", 0};

  void resetOSC() {
    client.open(port, addr);
    std::cout << "New OSC port Selected: \n" + port << std::endl;
    std::cout << addr << std::endl;
  }

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                           ImGuiWindowFlags_AlwaysAutoResize | ImGuiCond_Once;
  ImFont *bodyFont;
  ImFont *titleFont;

  /*
  Audio IO
  */
  std::string currentAudioDeviceOut;
  std::string currentAudioDeviceIn;

  static const int MAX_AUDIO_OUTS = 2;
  static const int MAX_AUDIO_INS = 2;

  std::array<unsigned int, MAX_AUDIO_OUTS> AudioChanIndexOut;
  std::array<unsigned int, MAX_AUDIO_INS> AudioChanIndexIn;

  bool isPaused = false;
  const int SAMPLE_RATE = 48000;
  double globalSamplingRate = SAMPLE_RATE;
  const int BLOCK_SIZE = 1024;

  int getLeadChannelOut() const { return AudioChanIndexOut[0]; }
  int getLeadChannelIn() const { return AudioChanIndexIn[0]; }

  void setOutChannels(int lead_channel, int max_possible_channels) {
    AudioChanIndexOut[0] = lead_channel;
    if (max_possible_channels == 1) {
      for (int i = 1; i < MAX_AUDIO_OUTS; i++) {
        AudioChanIndexOut[i] = lead_channel;
      }
    } else {
      // assert(lead_channel + (consts::MAX_AUDIO_OUTS) < max_possible_channels);
      for (int i = 1; i < MAX_AUDIO_OUTS; i++) {
        AudioChanIndexOut[i] = lead_channel + i;
      }
    }
  }
  void setInChannels(int lead_channel, int max_possible_channels) {
    AudioChanIndexIn[0] = lead_channel;
    if (max_possible_channels == 1) {
      for (int i = 1; i < MAX_AUDIO_OUTS; i++) {
        AudioChanIndexIn[i] = lead_channel;
      }
    } else {
      // assert(lead_channel + (consts::MAX_AUDIO_OUTS) < max_possible_channels);
      for (int i = 1; i < MAX_AUDIO_OUTS; i++) {
        AudioChanIndexIn[i] = lead_channel + i;
      }
    }
  }
  int getSampleRateIndex() {
    unsigned s_r = (unsigned)globalSamplingRate;
    switch (s_r) {
      case 44100:
        return 0;
      case 48000:
        return 1;
      case 88200:
        return 2;
      case 96000:
        return 3;
      default:
        return 0;
    }
  }

  // Audio input buffers
  unsigned int inBufferSize = 4096;
  RingBuffer inLeft{inBufferSize};
  RingBuffer inRight{inBufferSize};
  float driveForceLeft = 0;
  float driveForceRight = 0;
  gam::STFT stft{
    256,          // Window size
    256,          // Hop size; number of samples between transforms
    0,            // Pad size; number of zero-valued samples appended to window
    gam::HANN,    // Window type: BARTLETT, BLACKMAN, BLACKMAN_HARRIS,
                  //		HAMMING, HANN, WELCH, NYQUIST, or RECTANGLE
    gam::COMPLEX  // Format of frequency samples:
                  //		COMPLEX, MAG_PHASE, or MAG_FREQ
  };
  std::array<float, 129> fftBuffer = {};
  float fftDivision = 1;
  int fftIterator = 0;
};
