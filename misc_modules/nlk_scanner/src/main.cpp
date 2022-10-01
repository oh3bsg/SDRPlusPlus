#include <imgui.h>
#include <module.h>
#include <gui/gui.h>
#include <gui/style.h>
#include <core.h>
#include <signal_path/signal_path.h>
#include <spdlog/spdlog.h>
#include <iostream>
#include <fstream>
//#include <sstream>
#include <config.h>
//#include <filesystem>
//#include <json.hpp>

//using nlohmann::json;
using namespace std;
json bookmarks;
std::ofstream scan_log;
ConfigManager config;

SDRPP_MOD_INFO{
    /* Name:            */ "nlk_scanner",
    /* Description:     */ "NLK scanner for SDR++",
    /* Author:          */ "Ryzerth, Jarmo (OH3BSG)",
    /* Version:         */ 1, 0, 0,
    /* Max instances    */ -1
};

const char* demodModeListTxt = "NFM\0WFM\0AM\0DSB\0USB\0CW\0LSB\0RAW\0";

#define NUM_OF_TSKIP_FREQ   100
#define NUM_OF_BANKS        3

//#define SINGLE_
//#define MULTI_

typedef struct {
    double frq_start;
    double frq_stop;
    double frq_step;
    int frq_mode;
    bool selected;
} search_bank_str;

class NLK_ScannerModule : public ModuleManager::Instance {
public:
    NLK_ScannerModule(std::string name) {
        this->name = name;
        gui::menu.registerEntry(name, menuHandler, this, NULL);

        loadByName();
        refreshLists();

/*
        this->search_banks[0].frq_start = 118000000;
        this->search_banks[0].frq_stop = 137000000;
        this->search_banks[0].frq_step = 25000;
        this->search_banks[0].frq_mode = 2; // AM
        this->search_banks[1].frq_start = 450000000;
        this->search_banks[1].frq_stop = 470000000;
        this->search_banks[1].frq_step = 6250;
        this->search_banks[1].frq_mode = 0; // FM
        this->search_banks[2].frq_start = 150000000;
        this->search_banks[2].frq_stop = 180000000;
        this->search_banks[2].frq_step = 12500;
        this->search_banks[2].frq_mode = 0; // FM
*/
    }

    ~NLK_ScannerModule() {
        gui::menu.removeEntry(name);
        stop();
    }

    void postInit() {}

    void enable() {
        enabled = true;
    }

    void disable() {
        enabled = false;
    }

    bool isEnabled() {
        return enabled;
    }

private:
    static void menuHandler(void* ctx) {
        NLK_ScannerModule* _this = (NLK_ScannerModule*)ctx;
        float menuWidth = ImGui::GetContentRegionAvail().x;

        if (_this->running) { ImGui::BeginDisabled(); }
#ifdef SINGLE_        
        ImGui::LeftLabel("Start");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##start_freq_scanner", &_this->startFreq, 100.0, 100000.0, "%0.0f")) {
            _this->startFreq = round(_this->startFreq);
        }
        ImGui::LeftLabel("Stop");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##stop_freq_scanner", &_this->stopFreq, 100.0, 100000.0, "%0.0f")) {
            _this->stopFreq = round(_this->stopFreq);
        }
        ImGui::LeftLabel("Step");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##interval_scanner", &_this->interval, 100.0, 100000.0, "%0.0f")) {
            _this->interval = round(_this->interval);
        }
        ImGui::Combo("Mode##scanner_mode", &_this->frq_mode, demodModeListTxt);
#endif // SINGLE_
#ifdef MULTI_
        scanner_banks_ui(_this);
        ImGui::LeftLabel("Start_1");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##start_1_freq_scanner", &_this->search_banks[0].frq_start, 100.0, 100000.0, "%0.0f")) {
            _this->search_banks[0].frq_start = round(_this->search_banks[0].frq_start);
        }
        ImGui::LeftLabel("Stop_1");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##stop_1_freq_scanner", &_this->search_banks[0].frq_stop, 100.0, 100000.0, "%0.0f")) {
            _this->search_banks[0].frq_stop = round(_this->search_banks[0].frq_stop);
        }
        ImGui::LeftLabel("Step_1");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##interval_1_scanner", &_this->search_banks[0].frq_step, 100.0, 100000.0, "%0.0f")) {
            _this->search_banks[0].frq_step = round(_this->search_banks[0].frq_step);
        }
        ImGui::Combo("Mode##scanner_1_mode", &_this->search_banks[0].frq_mode, demodModeListTxt);

        ImGui::LeftLabel("Start_2");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##start_2_freq_scanner", &_this->search_banks[1].frq_start, 100.0, 100000.0, "%0.0f")) {
            _this->search_banks[1].frq_start = round(_this->search_banks[1].frq_start);
        }
        ImGui::LeftLabel("Stop_2");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##stop_2_freq_scanner", &_this->search_banks[1].frq_stop, 100.0, 100000.0, "%0.0f")) {
            _this->search_banks[1].frq_stop = round(_this->search_banks[1].frq_stop);
        }
        ImGui::LeftLabel("Step_2");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##interval_2_scanner", &_this->search_banks[1].frq_step, 100.0, 100000.0, "%0.0f")) {
            _this->search_banks[1].frq_step = round(_this->search_banks[1].frq_step);
        }
        ImGui::Combo("Mode##scanner_2_mode", &_this->search_banks[1].frq_mode, demodModeListTxt);

        ImGui::LeftLabel("Start_3");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##start_3_freq_scanner", &_this->search_banks[2].frq_start, 100.0, 100000.0, "%0.0f")) {
            _this->search_banks[2].frq_start = round(_this->search_banks[2].frq_start);
        }
        ImGui::LeftLabel("Stop_3");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##stop_3_freq_scanner", &_this->search_banks[2].frq_stop, 100.0, 100000.0, "%0.0f")) {
            _this->search_banks[2].frq_stop = round(_this->search_banks[2].frq_stop);
        }
        ImGui::LeftLabel("Step_3");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        if (ImGui::InputDouble("##interval_3_scanner", &_this->search_banks[2].frq_step, 100.0, 100000.0, "%0.0f")) {
            _this->search_banks[2].frq_step = round(_this->search_banks[2].frq_step);
        }
        ImGui::Combo("Mode##scanner_3_mode", &_this->search_banks[2].frq_mode, demodModeListTxt);
#endif // MULTI_

        scanner_banks_ui(_this);
        if (_this->running) { ImGui::EndDisabled(); } // ?????

        ImGui::LeftLabel("Level");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        ImGui::SliderFloat("##scanner_level", &_this->level, 0.0, 50.0);

        ImGui::LeftLabel("Delay");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        //ImGui::SliderFloat("##delay", &_this->delay, 0.0, 500.0);
        ImGui::SliderInt("##delay", &_this->delay, 0, 500);

        ImGui::LeftLabel("scan delay");
        ImGui::SetNextItemWidth(menuWidth - ImGui::GetCursorPosX());
        //ImGui::SliderFloat("##scan_delay", &_this->scan_delay, 100, 5000);
        ImGui::SliderInt("##scan_delay", &_this->scan_delay, 100, 5000);

        if (!_this->running) {
            if (ImGui::Button("Start##scanner_start", ImVec2(menuWidth, 0))) {
                _this->start();
            }
        }
        else {
            if (ImGui::Button("Stop##scanner_start", ImVec2(menuWidth, 0))) {
                _this->stop();
            }
        }

        if (ImGui::Button("T.Skip##scanner_tskip", ImVec2(menuWidth/2,0))) {
            _this->skip_add_temp(_this->frq);
            _this->receiving = false;
//            bool sql = true;
//            core::modComManager.callInterface(_this->selectedVFO, 5, &sql, NULL);
        }
        ImGui::SameLine();
        if (ImGui::Button("Skip##scanner_skip", ImVec2(menuWidth/2,0))) {
            
        }
        ImGui::Text(_this->metadata);
/*
        //Draw buttons on top of the list
        ImGui::BeginTable(("scanner_manager_btn_table" + _this->name).c_str(), 3);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        if (ImGui::Button(("Add##_scan_mgr_add_" + _this->name).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        }

        ImGui::TableSetColumnIndex(1);
//        if (selectedNames.size() == 0 && _this->selectedListName != "") { style::beginDisabled(); }
        if (ImGui::Button(("Remove##_freq_mgr_rem_" + _this->name).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            //_this->deleteBookmarksOpen = true;
        }
//        if (selectedNames.size() == 0 && _this->selectedListName != "") { style::endDisabled(); }

        ImGui::TableSetColumnIndex(2);
//        if (selectedNames.size() != 1 && _this->selectedListName != "") { style::beginDisabled(); }
        if (ImGui::Button(("Edit##_freq_mgr_edt_" + _this->name).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0))) {

            _this->editOpen = true;
            _this->editedBookmark = _this->bookmarks[selectedNames[0]];
            _this->editedBookmarkName = selectedNames[0];
            _this->firstEditedBookmarkName = selectedNames[0];

        }
//        if (selectedNames.size() != 1 && _this->selectedListName != "") { style::endDisabled(); }

        ImGui::EndTable();

        // Search bank list
        if (ImGui::BeginTable(("scan_manager_bkm_table" + _this->name).c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 200))) {
            ImGui::TableSetupColumn("Enable");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupScrollFreeze(2, 1);
            ImGui::TableHeadersRow();

            // dummy list
            for (int i=0; i<5;i++) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Checkbox("", &_this->enabled);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Name on bank");
            }
*/
/*
            for (auto& [name, bm] : _this->bookmarks) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImVec2 min = ImGui::GetCursorPos();

                if (ImGui::Selectable((name + "##_freq_mgr_bkm_name_" + _this->name).c_str(), &bm.selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_SelectOnClick)) {
                    // if shift or control isn't pressed, deselect all others
                    if (!ImGui::GetIO().KeyShift && !ImGui::GetIO().KeyCtrl) {
                        for (auto& [_name, _bm] : _this->bookmarks) {
                            if (name == _name) { continue; }
                            _bm.selected = false;
                        }
                    }
                }
                if (ImGui::TableGetHoveredColumn() >= 0 && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    applyBookmark(bm, gui::waterfall.selectedVFO);
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s %s", utils::formatFreq(bm.frequency).c_str(), demodModeList[bm.mode]);
                ImVec2 max = ImGui::GetCursorPos();
            }
*/
//            ImGui::EndTable();
//        }

    }

    void start() {
        if (running) { return; }
        //current = next_start();
        next_bank(current, stopFreq, interval, frq_mode);
        //current = startFreq + (gui::waterfall.getViewBandwidth()/2.0);
        running = true;
        receiving = false;
        workerThread = std::thread(&NLK_ScannerModule::worker, this);
        //stream = sigpath::sinkManager.getStreams(selectedVFO);
        scan_log.open("scan_log.txt", ios::out | ios::app | ios::binary);
        scan_log << "**** Start scanning ****\n";
    }

    void stop() {
        if (!running) { return; }
        running = false;
        if (workerThread.joinable()) {
            workerThread.join();
        }
        scan_log << "**** Stop scanning ****\n";
        scan_log.close();
    }

    void worker() {
        // 10Hz scan loop
        while (running) {
            if (receiving) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                // get FFT data
                int dataWidth = 0;
                float* data = gui::waterfall.acquireLatestFFT(dataWidth);

                // Get gather waterfall data
                double wfCenter = gui::waterfall.getViewOffset() + gui::waterfall.getCenterFrequency();
                double wfWidth = gui::waterfall.getViewBandwidth();
                double wfStart = wfCenter - (wfWidth / 2.0);
                double wfEnd = wfCenter + (wfWidth / 2.0);

                if ((data[rec_i]-data[rec_i-1]) > (level*0.6)) {
                    rec_timeout = 0;
                    bool sql = false;
                    core::modComManager.callInterface(selectedVFO, 5, &sql, NULL);
                    autoskip_timeout++;
                    if (autoskip_timeout >= 600) {
                        autoskip_timeout = 0;
                        receiving = false;
                        skip_add_temp(frq);
                    }
                }
                else {
                    rec_timeout++;
                    // Laita squelch päälle
                    bool sql = true;
                    core::modComManager.callInterface(selectedVFO, 5, &sql, NULL);
                    
                    //if (rec_timeout > 10) {
                    if (rec_timeout > (scan_delay/100)) {
                        receiving = false;
                        autoskip_timeout = 0;
                    }
                }

                // Release FFT Data
                gui::waterfall.releaseLatestFFT();
            }
            else {  // scanning
                bool sql = true;
                core::modComManager.callInterface(selectedVFO, 5, &sql, NULL);

                tuner::centerTuning(selectedVFO, current);
                std::this_thread::sleep_for(std::chrono::milliseconds((long)delay));

                // get FFT data
                int dataWidth = 0;
                float* data = gui::waterfall.acquireLatestFFT(dataWidth);

                // Get gather waterfall data
                double wfCenter = gui::waterfall.getViewOffset() + gui::waterfall.getCenterFrequency();
                double wfWidth = gui::waterfall.getViewBandwidth();
                double wfStart = wfCenter - (wfWidth / 2.0);
                double wfEnd = wfCenter + (wfWidth / 2.0);

                double signalWidth = dataWidth;
                if ((current + (gui::waterfall.getViewBandwidth()/2.0)) > stopFreq) {
                    // calculate new signalWidth
                    double f = stopFreq-(current - gui::waterfall.getViewBandwidth()/2);
                    //signalWidth = (f/gui::waterfall.getViewBandwidth()) * signalWidth;
                    signalWidth *= f/gui::waterfall.getViewBandwidth();
                }

                // search signal
                for  (int i=1; i<signalWidth; i++) {
                    if ((data[i]-data[i-1]) > level) {
                        frq = round((wfStart+(i*(wfWidth / (double)dataWidth)))/interval)*interval;
                        if (!skip(frq)) {
                            int mode = frq_mode;
                            core::modComManager.callInterface(selectedVFO, 1 /*RADIO_IFACE_CMD_SET_MODE*/, &mode, NULL);
                            // Laita squelch pois päältä (5)
                            bool sql = false;
                            core::modComManager.callInterface(selectedVFO, 5, &sql, NULL);

                            tuner::tune(tuner::TUNER_MODE_NORMAL, selectedVFO, frq);
                            int mega = (int)(frq/1000000);
                            int kilo = ((int)frq%1000000)/1000;
                            char name[30];
                            get_name(frq, name);
                            sprintf(metadata, "%d.%03d MHz - %s", mega, kilo, name);
                            std::ofstream meta("metadata.txt");
                            meta << metadata;
                            scan_log << metadata << "\n";
                            receiving = true;
                            rec_i = i;
                            break;
                        }
                    }
                }

                // Release FFT Data
                gui::waterfall.releaseLatestFFT();

                // current++
                if (!receiving) {
                    current += gui::waterfall.getViewBandwidth();

                    if ((current - (gui::waterfall.getViewBandwidth()/2.0)) > stopFreq) {
                        next_bank(current, stopFreq, interval, frq_mode);
                        //current = startFreq + (gui::waterfall.getViewBandwidth()/2.0); 
                    }
                }
            }
        }
    }

    bool skip(double frq) {
        // Check temporary skip list
        for (int i=0; i<NUM_OF_TSKIP_FREQ; i++) {
            if (this->TSkipFrqs[i] == frq) return true;
        }

        return false;
    }

    void skip_add_temp(double frq) {
        for (int i=0; i<NUM_OF_TSKIP_FREQ; i++) {
            if (this->TSkipFrqs[i] == 0) {
                this->TSkipFrqs[i] = frq;
                char msg[45];
                //sprintf(msg, "%d   -  TSkipped\n", frq);
                sprintf(msg, "%.3lf MHz - TSkipped\n", frq / 1000000.0);
                scan_log << msg;
                break;
            }
        }
    }

    void get_name(double frq, char *name) {
        sprintf(name, "UNID");
        for (auto [bname, b] : bookmarks["lists"]["General"]["bookmarks"].items()) {
            double t = bookmarks["lists"]["General"]["bookmarks"][bname]["frequency"];
            if ((int)frq == (int(t))) {
                sprintf(name, "%s", bname);
            }
        }
    }

/*
    double next_start(void) {
        double start_frq;
        static int i = 0;
        
        start_frq = this->search_banks[i++].frq_start + (gui::waterfall.getViewBandwidth()/2.0);
        if (i > 1) i = 0;
        return start_frq;
    }
*/

    void next_bank(double& startF, double& stopF, double& step, int& mode) {
        static int i = 0;

        if (this->banks[listNames[i]].selected == true) {
            startF = this->banks[listNames[i]].frq_start + (gui::waterfall.getViewBandwidth()/2.0);
            stopF = this->banks[listNames[i]].frq_stop;
            step = this->banks[listNames[i]].frq_step;
            mode = this->banks[listNames[i]].frq_mode;
        }

        i++;
        //if (i >= NUM_OF_BANKS) i = 0;
        if (i >= listNames.size()) i = 0;
    }

// **************************************************************************
// Search bank handling
// **************************************************************************

    bool bankEditDialog() {
        bool open = true;
        //gui::mainWindow.lockWaterfallControls = true;

        std::string id = "Edit##freq_manager_edit_popup_"; // + name;
        ImGui::OpenPopup(id.c_str());

        char nameBuf[1024];
        strcpy(nameBuf, editedBankName.c_str());

        if (ImGui::BeginPopup(id.c_str(), ImGuiWindowFlags_NoResize)) {
            ImGui::BeginTable(("freq_manager_edit_table"/* + name*/)/*.c_str()*/, 2);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::LeftLabel("Bank name");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(200);
            if (ImGui::InputText(("##freq_manager_edit_name"/* + name*/)/*.c_str()*/, nameBuf, 1023)) {
                editedBankName = nameBuf;
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::LeftLabel("Start Frequency");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(200);
            ImGui::InputDouble(("##freq_manager_edit_start_freq"/* + name*/)/*.c_str()*/, &editedBank.frq_start);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::LeftLabel("Stop Frequency");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(200);
            ImGui::InputDouble(("##freq_manager_edit_stop_freq"/* + name*/)/*.c_str()*/, &editedBank.frq_stop);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::LeftLabel("Step");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(200);
            ImGui::InputDouble(("##freq_manager_edit_bw"/* + name*/)/*.c_str()*/, &editedBank.frq_step);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::LeftLabel("Mode");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(200);
            ImGui::Combo(("##freq_manager_edit_mode"/* + name*/)/*.c_str()*/, &editedBank.frq_mode, demodModeListTxt);

            ImGui::EndTable();

            /*
            bool applyDisabled = (strlen(nameBuf) == 0) || (bookmarks.find(editedBookmarkName) != bookmarks.end() && editedBookmarkName != firstEditedBookmarkName);
            if (applyDisabled) { style::beginDisabled(); }
            */
            if (ImGui::Button("Apply")) {
                open = false;

                // If editing, delete the original one
                //if (editOpen) {
                    //bookmarks.erase(firstEditedBookmarkName);
                //}
                banks[editedBankName] = editedBank;

                saveByName(/*selectedListName*/);
            }
            
            //if (applyDisabled) { style::endDisabled(); }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                open = false;
            }
            ImGui::EndPopup();
        }
        return open;
    }

    void refreshLists() {
        listNames.clear();
        //listNamesTxt = "";

        config.acquire();
        for (auto [_name, list] : config.conf["banks"].items()) {
            listNames.push_back(_name);
            //listNamesTxt += _name;
            //listNamesTxt += '\0';
        }
        config.release();
    }


    void loadByName(void/*std::string listName*/) {
        banks.clear();
/*
        if (std::find(listNames.begin(), listNames.end(), listName) == listNames.end()) {
            selectedListName = "";
            selectedListId = 0;
            loadFirst();
            return;
        }
        selectedListId = std::distance(listNames.begin(), std::find(listNames.begin(), listNames.end(), listName));
        selectedListName = listName;
*/
        config.acquire();
        for (auto [name, bank] : config.conf["banks"].items()) {
            search_bank_str _bank;
            _bank.frq_start = bank["start_frequency"];
            _bank.frq_stop = bank["stop_frequency"];
            _bank.frq_step = bank["step"];
            _bank.frq_mode = bank["mode"];
            banks[name] = _bank;
        }
        config.release();
    }

    void saveByName(void) {
        config.acquire();
        config.conf["banks"] = json::object();
        for (auto [bankName, bank] : banks) {
            config.conf["banks"][bankName]["start_frequency"] = bank.frq_start;
            config.conf["banks"][bankName]["stop_frequency"] = bank.frq_stop;
            config.conf["banks"][bankName]["step"] = bank.frq_step;
            config.conf["banks"][bankName]["mode"] = bank.frq_mode;
        }
        //refreshWaterfallBookmarks(false);
        config.release(true);
    }

    static void scanner_banks_ui(void* ctx) {
        NLK_ScannerModule* _this = (NLK_ScannerModule*)ctx;
//        FrequencyManagerModule* _this = (FrequencyManagerModule*)ctx;
        float menuWidth = ImGui::GetContentRegionAvail().x;

        // TODO: Replace with something that won't iterate every frame
        std::vector<std::string> selectedNames;
/*
        for (auto& [name, bm] : _this->bookmarks) {
            if (bm.selected) { selectedNames.push_back(name); }
        }

        float lineHeight = ImGui::GetTextLineHeightWithSpacing();

        float btnSize = ImGui::CalcTextSize("Rename").x + 8;
        ImGui::SetNextItemWidth(menuWidth - 24 - (2 * lineHeight) - btnSize);
        if (ImGui::Combo(("##freq_manager_list_sel" + _this->name).c_str(), &_this->selectedListId, _this->listNamesTxt.c_str())) {
            _this->loadByName(_this->listNames[_this->selectedListId]);
            config.acquire();
            config.conf["selectedList"] = _this->selectedListName;
            config.release(true);
        }
        ImGui::SameLine();
        if (_this->listNames.size() == 0) { style::beginDisabled(); }
        if (ImGui::Button(("Rename##_freq_mgr_ren_lst_" + _this->name).c_str(), ImVec2(btnSize, 0))) {
            _this->firstEditedListName = _this->listNames[_this->selectedListId];
            _this->editedListName = _this->firstEditedListName;
            _this->renameListOpen = true;
        }
        if (_this->listNames.size() == 0) { style::endDisabled(); }
        ImGui::SameLine();
        if (ImGui::Button(("+##_freq_mgr_add_lst_" + _this->name).c_str(), ImVec2(lineHeight, 0))) {
            // Find new unique default name
            if (std::find(_this->listNames.begin(), _this->listNames.end(), "New List") == _this->listNames.end()) {
                _this->editedListName = "New List";
            }
            else {
                char buf[64];
                for (int i = 1; i < 1000; i++) {
                    sprintf(buf, "New List (%d)", i);
                    if (std::find(_this->listNames.begin(), _this->listNames.end(), buf) == _this->listNames.end()) { break; }
                }
                _this->editedListName = buf;
            }
            _this->newListOpen = true;
        }
        ImGui::SameLine();
        if (_this->selectedListName == "") { style::beginDisabled(); }
        if (ImGui::Button(("-##_freq_mgr_del_lst_" + _this->name).c_str(), ImVec2(lineHeight, 0))) {
            _this->deleteListOpen = true;
        }
        if (_this->selectedListName == "") { style::endDisabled(); }

        // List delete confirmation
        if (ImGui::GenericDialog(("freq_manager_del_list_confirm" + _this->name).c_str(), _this->deleteListOpen, GENERIC_DIALOG_BUTTONS_YES_NO, [_this]() {
                ImGui::Text("Deleting list named \"%s\". Are you sure?", _this->selectedListName.c_str());
            }) == GENERIC_DIALOG_BUTTON_YES) {
            config.acquire();
            config.conf["lists"].erase(_this->selectedListName);
            _this->refreshWaterfallBookmarks(false);
            config.release(true);
            _this->refreshLists();
            _this->selectedListId = std::clamp<int>(_this->selectedListId, 0, _this->listNames.size());
            if (_this->listNames.size() > 0) {
                _this->loadByName(_this->listNames[_this->selectedListId]);
            }
            else {
                _this->selectedListName = "";
            }
        }

        if (_this->selectedListName == "") { style::beginDisabled(); }
*/
        //Draw buttons on top of the list
        ImGui::BeginTable(("freq_manager_btn_table"/* + _this->name*/)/*.c_str()*/, 3);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        if (ImGui::Button(("Add##_freq_mgr_add_"/* + _this->name*/)/*.c_str()*/, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            _this->createOpen = true;

            // defaults
            _this->editedBank.frq_start = 118000000.0f;
            _this->editedBank.frq_stop = 137000000.0f;
            _this->editedBank.frq_step = 25000.0f;
            _this->editedBank.frq_mode = 2;
            // Find new unique default name
            //if (_this->bookmarks.find("New Bookmark") == _this->bookmarks.end()) {
                _this->editedBankName = "New Bank";
            //}
            //else {
            //    char buf[64];
            //    for (int i = 1; i < 1000; i++) {
            //        sprintf(buf, "New Bank (%d)", i);
            //        if (_this->bookmarks.find(buf) == _this->bookmarks.end()) { break; }
            //    }
            //    _this->editedBankName = buf;
            //}
        }

        ImGui::TableSetColumnIndex(1);
        //if (selectedNames.size() == 0 && _this->selectedListName != "") { style::beginDisabled(); }
        if (ImGui::Button(("Remove##_freq_mgr_rem_"/* + _this->name*/)/*.c_str()*/, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            //_this->deleteBookmarksOpen = true;
        }
        //if (selectedNames.size() == 0 && _this->selectedListName != "") { style::endDisabled(); }
        ImGui::TableSetColumnIndex(2);
        //if (selectedNames.size() != 1 && _this->selectedListName != "") { style::beginDisabled(); }
        if (ImGui::Button(("Edit##_freq_mgr_edt_"/* + _this->name*/)/*.c_str()*/, ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            _this->editOpen = true;
            _this->editedBank = _this->banks[selectedNames[0]];
            _this->editedBankName = selectedNames[0];
            //_this->firstEditedBookmarkName = selectedNames[0];
        }
        //if (selectedNames.size() != 1 && _this->selectedListName != "") { style::endDisabled(); }

        ImGui::EndTable();
/*
        // Bank delete confirm dialog
        // List delete confirmation
        if (ImGui::GenericDialog(("freq_manager_del_list_confirm" + _this->name).c_str(), _this->deleteBookmarksOpen, GENERIC_DIALOG_BUTTONS_YES_NO, [_this]() {
                ImGui::TextUnformatted("Deleting selected bookmaks. Are you sure?");
            }) == GENERIC_DIALOG_BUTTON_YES) {
            for (auto& _name : selectedNames) { _this->bookmarks.erase(_name); }
            _this->saveByName(_this->selectedListName);
        }
*/
        // Bank list
        if (ImGui::BeginTable(("freq_manager_bkm_table" + _this->name).c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0, 200))) {
            ImGui::TableSetupColumn("Bank name");
            ImGui::TableSetupColumn("Freq range");
            ImGui::TableSetupScrollFreeze(2, 1);
            ImGui::TableHeadersRow();
            
            for (auto& [name, bank] : _this->banks) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImVec2 min = ImGui::GetCursorPos();
                
                if (ImGui::Selectable((name + "##_freq_mgr_bkm_name_" + _this->name).c_str(), &bank.selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_SelectOnClick)) {
                    // if shift or control isn't pressed, deselect all others
                    if (!ImGui::GetIO().KeyShift && !ImGui::GetIO().KeyCtrl) {
                        for (auto& [_name, _bank] : _this->banks) {
                            if (name == _name) { continue; }
                            _bank.selected = false;
                        }
                    }
                }
                
                /*
                if (ImGui::TableGetHoveredColumn() >= 0 && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    applyBookmark(bm, gui::waterfall.selectedVFO);
                }
                */
                //ImGui::TableSetColumnIndex(1);
                //ImGui::Text("%s %s", utils::formatFreq(bm.frequency).c_str(), demodModeList[bm.mode]);
                //ImGui::Text("%s", name);
                ImVec2 max = ImGui::GetCursorPos();
            }
            
            ImGui::EndTable();
        }
/*

        if (selectedNames.size() != 1 && _this->selectedListName != "") { style::beginDisabled(); }
        if (ImGui::Button(("Apply##_freq_mgr_apply_" + _this->name).c_str(), ImVec2(menuWidth, 0))) {
            FrequencyBookmark& bm = _this->bookmarks[selectedNames[0]];
            applyBookmark(bm, gui::waterfall.selectedVFO);
            bm.selected = false;
        }
        if (selectedNames.size() != 1 && _this->selectedListName != "") { style::endDisabled(); }

        //Draw import and export buttons
        ImGui::BeginTable(("freq_manager_bottom_btn_table" + _this->name).c_str(), 2);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        if (ImGui::Button(("Import##_freq_mgr_imp_" + _this->name).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)) && !_this->importOpen) {
            _this->importOpen = true;
            _this->importDialog = new pfd::open_file("Import bookmarks", "", { "JSON Files (*.json)", "*.json", "All Files", "*" }, true);
        }

        ImGui::TableSetColumnIndex(1);
        if (selectedNames.size() == 0 && _this->selectedListName != "") { style::beginDisabled(); }
        if (ImGui::Button(("Export##_freq_mgr_exp_" + _this->name).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0)) && !_this->exportOpen) {
            _this->exportedBookmarks = json::object();
            config.acquire();
            for (auto& _name : selectedNames) {
                _this->exportedBookmarks["bookmarks"][_name] = config.conf["lists"][_this->selectedListName]["bookmarks"][_name];
            }
            config.release();
            _this->exportOpen = true;
            _this->exportDialog = new pfd::save_file("Export bookmarks", "", { "JSON Files (*.json)", "*.json", "All Files", "*" }, true);
        }
        if (selectedNames.size() == 0 && _this->selectedListName != "") { style::endDisabled(); }
        ImGui::EndTable();

        if (ImGui::Button(("Select displayed lists##_freq_mgr_exp_" + _this->name).c_str(), ImVec2(menuWidth, 0))) {
            _this->selectListsOpen = true;
        }

        if (_this->selectedListName == "") { style::endDisabled(); }
*/
        if (_this->createOpen) {
            _this->createOpen = _this->bankEditDialog();
        }

        if (_this->editOpen) {
            _this->editOpen = _this->bankEditDialog();
        }
/*

        if (_this->newListOpen) {
            _this->newListOpen = _this->newListDialog();
        }

        if (_this->renameListOpen) {
            _this->renameListOpen = _this->newListDialog();
        }

        if (_this->selectListsOpen) {
            _this->selectListsOpen = _this->selectListsDialog();
        }

        // Handle import and export
        if (_this->importOpen && _this->importDialog->ready()) {
            _this->importOpen = false;
            std::vector<std::string> paths = _this->importDialog->result();
            if (paths.size() > 0 && _this->listNames.size() > 0) {
                _this->importBookmarks(paths[0]);
            }
            delete _this->importDialog;
        }
        if (_this->exportOpen && _this->exportDialog->ready()) {
            _this->exportOpen = false;
            std::string path = _this->exportDialog->result();
            if (path != "") {
                _this->exportBookmarks(path);
            }
            delete _this->exportDialog;
        }
*/
    }

    std::string name;
    bool enabled = true;
    
    bool running = false;
    std::string selectedVFO = "Radio";
    double startFreq = 118000000.0;
    double stopFreq = 137000000.0;
    double interval = 25000.0;
    double current = 88000000.0;
    double frq;
    double TSkipFrqs[NUM_OF_TSKIP_FREQ] = {0};
    int frq_mode;
    float level = 23.0;
    int delay = 100.0;
    int scan_delay = 500;
    int rec_i;
    int rec_timeout = 100;
    int autoskip_timeout = 0;
    bool receiving = false;
    bool tuning = false;
    bool scanUp = true;
    bool check = false;
    bool searching = false;
    bool check_booli = false;
    bool createOpen = false;
    bool editOpen = false;
    char metadata[45] = "";
    //json bookmarks;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastSignalTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTuneTime;
    std::thread workerThread;
    std::mutex scanMtx;
    //EventHandler<ImGui::WaterFall::InputHandlerArgs> inputHandler;
    SinkManager::Stream* stream;
    dsp::stream<dsp::stereo_t>* audioInput = NULL;
    search_bank_str search_banks[NUM_OF_BANKS];

    std::map<std::string, search_bank_str> banks;
    std::string editedBankName = "";
    search_bank_str editedBank;
    std::vector<std::string> listNames;
};

MOD_EXPORT void _INIT_() {
    std::ifstream file("frequency_manager_config.json");
    file >> bookmarks;
    file.close();

    json def = json({});
    def["banks"] = json::object();
    def["skip frequencies"] = json::object();

    config.setPath(core::args["root"].s() + "/scanner_config.json");
    config.load(def);
    config.enableAutoSave();

/*
    config.acquire();
    for (auto [listName, list] : config.conf["banks"].items()) {
        //if (list.contains("bookmarks") && list.contains("showOnWaterfall") && list["showOnWaterfall"].is_boolean()) { continue; }
        json newList;
        newList = json::object();
        //newList["showOnWaterfall"] = true;
        //newList["bookmarks"] = list;
        config.conf["banks"][listName] = newList;
    }
    config.release(true);
*/
}

MOD_EXPORT ModuleManager::Instance* _CREATE_INSTANCE_(std::string name) {
    return new NLK_ScannerModule(name);
}

MOD_EXPORT void _DELETE_INSTANCE_(void* instance) {
    delete (NLK_ScannerModule*)instance;
}

MOD_EXPORT void _END_() {
    // Nothing here
}
