#include <imgui.h>
#include <module.h>
//#include <gui/gui.h>
#include <core.h>
#include <chrono>
#include <thread>
#include <functional>
#include <gui/widgets/waterfall.h>
#include <signal_path/sink.h>
//#include <signal_path/signal_path.h>
//#include <dsp/processing.h>
//#include <radio_interface.h>

// https://stackoverflow.com/questions/21057676/need-to-call-a-function-at-periodic-time-intervals-in-c
//
//

SDRPP_MOD_INFO{
    /* Name:            */ "search_scan",
    /* Description:     */ "Search and scan",
    /* Author:          */ "OH3BSG",
    /* Version:         */ 0, 1, 0,
    /* Max instances    */ -1
};

const char* demodModeListTxt = "NFM\0WFM\0AM\0DSB\0USB\0CW\0LSB\0RAW\0";

typedef struct {
    double frq_start;
    double frq_stop;
    double frq_step;
    int frq_mode;
    double samplerate;
    double detection_level;
    double release_level;
}  frq_scan_info_str;

/*
void do_something()
    {
        printf("do_something\n");
    }
*/
class NlkModule : public ModuleManager::Instance {
public:
    NlkModule(std::string name) {
        printf("\nNlkModule()\n");
        this->name = name;
/*
        fftRedrawHandler.ctx = this;
        fftRedrawHandler.handler = fftRedraw;
*/
        inputHandler.ctx = this;
        inputHandler.handler = fftInput;

        //gui::menu.registerEntry(name, menuHandler, this, NULL);
//        gui::waterfall.onFFTRedraw.bindHandler(&fftRedrawHandler);
        //gui::waterfall.onInputProcess.bindHandler(&inputHandler);

        this->frq.frq_start = 118000000;
        this->frq.frq_stop = 137000000;
        this->frq.frq_step = 25000;
        this->frq.frq_mode = 0;
        this->frq.samplerate = 2400000;
        this->frq.detection_level = 20;
        this->frq.release_level = 15;

        timer_start(timer_handler, 100, this);

        std::thread([this]()
        { 
            while (true)
                { 
                }
        }).detach();

    }

    ~NlkModule() {
        printf("\n~NlkModule()\n");
        //gui::menu.removeEntry(name);
        //gui::waterfall.onInputProcess.unbindHandler(&inputHandler);
    }

    void postInit() {}

    void enable() {
        printf("NLK enable()\n");
        enabled = true;
    }

    void disable() {
        enabled = false;
    }

    bool isEnabled() {
        return enabled;
    }

private:

//    EventHandler<ImGui::WaterFall::FFTRedrawArgs> fftRedrawHandler;
    EventHandler<ImGui::WaterFall::InputHandlerArgs> inputHandler;

    static void menuHandler(void* ctx) {
        NlkModule* _this = (NlkModule*)ctx;
        
        ImGui::Text("Moikka SDR++, Olen %s", _this->name.c_str());
        ImGui::Separator();
        ImGui::Checkbox("Enable", &_this->enabled);
        ImGui::Text("Start freq"); ImGui::SameLine(); ImGui::InputDouble("Start", &_this->frq.frq_start);
        ImGui::Text("Stop freq"); ImGui::SameLine(); ImGui::InputDouble("Stop", &_this->frq.frq_stop);
        ImGui::Text("Step"); ImGui::SameLine(); ImGui::InputDouble("Step", &_this->frq.frq_step);
        //ImGui::Text("Mode"); ImGui::SameLine(); ImGui::InputInt("Mode", &_this->frq.frq_mode);
        //ImGui::Combo(("##freq_manager_edit_mode" + name).c_str(), &editedBookmark.mode, demodModeListTxt);
        ImGui::Combo("Mode", &_this->frq.frq_mode, demodModeListTxt);
    }

    static void timer_handler(NlkModule* _this) {
        static double mid_frq = _this->frq.frq_start + (_this->frq.samplerate/2);

        //SinkManager* sm = new SinkManager;
        //SinkManager::Stream* stream = sm->getStreams(gui::waterfall.selectedVFO);

        if (mid_frq >= _this->frq.frq_stop) {
            mid_frq = _this->frq.frq_start + (_this->frq.samplerate/2);
        }

        if (_this->isEnabled()) {
            if (_this->receive) {
                //if (stream != NULL) {
                //    stream->volumeAjust.setMuted(false);
                //}
                /*
                if (gui::waterfall.selectedVFOSNR < _this->frq.release_level) {
                    _this->receive = false;
                } 
                */
            }
            else {
                //SinkManager::Stream* stream = sigpath::sinkManager.getStreams(gui::waterfall.selectedVFO);
                //stream->volumeAjust.setMuted(true);

                printf("mid: %f\n", mid_frq);
                //if (stream != NULL) {
                //    stream->volumeAjust.setMuted(true);
                //}
                //gui::waterfall.setCenterFrequency(mid_frq);
                //gui::waterfall.centerFreqMoved = true;
                mid_frq += _this->frq.samplerate;
            }
        }

/*
        static int mid_frq = _this->frq.frq_start + (_this->frq.samplerate/2);

        if (mid_frq >= _this->frq.frq_stop) {
            mid_frq = _this->frq.frq_start + (_this->frq.samplerate/2);
        }

        if (_this->isEnabled()) {
            gui::waterfall.setCenterFrequency((double)mid_frq);
            gui::waterfall.centerFreqMoved = true;

            std::string vfoName = gui::waterfall.selectedVFO;
            int mode = 2;
            core::modComManager.interfaceExists(vfoName);
            core::modComManager.callInterface(vfoName, 1 /RADIO_IFACE_CMD_SET_MODE/, &mode, NULL);
            float f = mid_frq - (_this->frq.samplerate/2);

//            float *fft;
//            auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
//            std::this_thread::sleep_until(x);
//            fft = gui::waterfall.getFFTBuffer();
//            printf("FFT: %x %f\n", (uint8_t *)fft, gui::waterfall.getCenterFrequency());
//            printf("FFT: %x\n", *fft+546);
//            gui::waterfall.pushFFT();


            for (int i=0; i<90; i++) {
                //float dummy;
                //float snr;

                //calculateVFOSignalInfo(fft, vfos[selectedVFO], &dummy, &snr);
                tuner::tune(tuner::TUNER_MODE_NORMAL, vfoName, (double)f);
                auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(40);
                std::this_thread::sleep_until(x);
                float snr = gui::waterfall.selectedVFOSNR;
                if (snr > 20.0) {
                    printf("f %f  SNR: %f\n", f, snr);
                }
                f += 25000;
            }

        }
        //mid_frq += _this->frq.samplerate;
*/
    }

    void timer_start(std::function<void(NlkModule*)> func, unsigned int interval, NlkModule* _this)
    {
        std::thread([func, interval, _this]()
        { 
            while (true)
                { 
                auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
                func(_this);
                std::this_thread::sleep_until(x);
                }
        }).detach();
    }


    static void fftInput(ImGui::WaterFall::InputHandlerArgs args, void* ctx) {
        NlkModule* _this = (NlkModule*)ctx;
        static bool on_going = false;
        float *fft;
        double viewBandwidth;
        int len;

        if (_this->isEnabled() && !_this->receive) {
            //fft = gui::waterfall.getLatestFFT(&len);
            //viewBandwidth = gui::waterfall.getViewBandwidth();

            for (int i = 1; i < len; i++) {
                if (((fft[i]-fft[i-1]) > _this->frq.detection_level) && (!_this->receive)) {
                    _this->freq = round((args.lowFreq+(i*(viewBandwidth / (double)len)))/25000)*25000;
                    printf("%f\n", fft[i]-fft[i-1]);
                    _this->receive = true;
 
                    // Frequency blacklist
                    if ( _this->freq == 135950000) _this->receive = false;
                    if ( _this->freq == 133550000) _this->receive = false;
                    if ( _this->freq == 121500000) _this->receive = false;
                    if ( _this->freq == 129300000) _this->receive = false;
                }
            }
            //on_going = false;
            if (_this->receive) {
                //SinkManager::Stream* stream = sigpath::sinkManager.getStreams(gui::waterfall.selectedVFO);
                //stream->volumeAjust.setMuted(false);

                //std::string vfoName = gui::waterfall.selectedVFO;
                int mode = _this->frq.frq_mode;
                //core::modComManager.interfaceExists(vfoName);
                //core::modComManager.callInterface(vfoName, 1 /*RADIO_IFACE_CMD_SET_MODE*/, &mode, NULL);
                //tuner::tune(tuner::TUNER_MODE_NORMAL, vfoName, _this->freq);
                //_this->receive = false;
                printf("Receiving %f\n", _this->freq);
            }   
        }
    }

/*
    static void fftRedraw(ImGui::WaterFall::FFTRedrawArgs args, void* ctx) {
        NlkModule* _this = (NlkModule*)ctx;
 
        if (_this->isEnabled()) {
            printf("  LowFrq = %f  HighFrq = %f\n", args.lowFreq, args.highFreq);
            float *fft;
            fft = gui::waterfall.getFFTBuffer();
            printf("FFT: %x %f\n", (uint8_t *)fft, gui::waterfall.getCenterFrequency());
            printf("FFT: %x\n", *fft+29000);
            gui::waterfall.pushFFT();
        }
    }
*/
    std::string name;
    bool enabled = false;
    bool receive = false;
    double freq;
    frq_scan_info_str frq;
};

MOD_EXPORT void _INIT_() {
    // Nothing here
}

MOD_EXPORT ModuleManager::Instance* _CREATE_INSTANCE_(std::string name) {
    return new NlkModule(name);
}

MOD_EXPORT void _DELETE_INSTANCE_(void* instance) {
    delete (NlkModule*)instance;
}

MOD_EXPORT void _END_() {
    // Nothing here
}

