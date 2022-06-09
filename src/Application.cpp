// ImGui - standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <Windows.h>
#include "../Dependencies/discord/include/discord_register.h"
#include "../Dependencies/discord/include/discord_rpc.h"
#include <iostream>
#include <thread>
#include "../Dependencies/BASS/api.h"
#include "../Dependencies/BASS/Sounds.h"
#include <string>
#include <Psapi.h>
#include <chrono>
#include <thread>
#define STB_IMAGE_IMPLEMENTATION
#include "../Dependencies/stb_image.h"
#include "../Dependencies/BASS/bass.h"
#include "ShlObj.h"
#include <vector>
#include "../Dependencies/BASS/string_obfuscation.h"

std::vector<std::string> get_all_files_names_within_folder(std::string folder)
{
    std::vector<std::string> names;
    std::string search_path = folder + "/*.mp3";
    WIN32_FIND_DATA fd;
    wchar_t wtext_2[300] = { };
    mbstowcs(wtext_2, search_path.c_str(), search_path.length());
    LPWSTR ptr_2 = wtext_2;
    HANDLE hFind = ::FindFirstFile(ptr_2, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            // read all (real) files in current folder
            // , delete '!' read other 2 default folder . and ..
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::wstring ws(fd.cFileName);
                std::string str(ws.begin(), ws.end());
                names.push_back(str);
            }
        } while (::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
    return names;
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

void Initialize(std::string client_id) {
    DiscordEventHandlers Handler;
    memset(&Handler, 0, sizeof(Handler));
    Discord_Initialize(client_id.c_str(), &Handler, TRUE, NULL);
}

void Update(std::string state, std::string details, std::string LargeImageKey, std::string SmallImageKey, std::string LargeImageText, std::string SmallImageText, long long time, bool enable_firstbtn, bool enable_secondbtn, std::string Button1Label, std::string Button1Url, std::string Button2Label, std::string Button2Url) {
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    discordPresence.state = state.c_str();
    discordPresence.details = details.c_str();
    discordPresence.startTimestamp = /*time(0)*/ time;
    discordPresence.largeImageKey = LargeImageKey.c_str();
    discordPresence.smallImageKey = SmallImageKey.c_str();
    discordPresence.largeImageText = LargeImageText.c_str();
    discordPresence.smallImageText = SmallImageText.c_str();
    discordPresence.enable_firstbtn = enable_firstbtn;
    discordPresence.enable_secondbtn = enable_secondbtn;
    discordPresence.Button1Label = Button1Label.c_str();
    discordPresence.Button1Url = Button1Url.c_str();
    discordPresence.Button2Label = Button2Label.c_str();
    discordPresence.Button2Url = Button2Url.c_str();
    Discord_UpdatePresence(&discordPresence);

}

wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
    wchar_t* wString = new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}

void HideConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

void Shutdown()
{
    Discord_Shutdown();
}


inline bool exists_test(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

using namespace std::chrono_literals;
char buffer_add_radio[300];
static std::pair<std::string, char> channels[] = {
    __(" "),
    __("https://streams.ilovemusic.de/iloveradio16.mp3?hadpreroll"), // Greatest Hits
    __("https://streams.ilovemusic.de/iloveradio2.mp3?hadpreroll"), // Dance Hits
    __("https://streams.ilovemusic.de/iloveradio6.mp3?hadpreroll"), // Best German rap
    __("http://streams.ilovemusic.de/iloveradio10.mp3?hadpreroll"), // Chill 
    __("https://streams.ilovemusic.de/iloveradio109.mp3?hadpreroll"), // Top 100
    __("https://streams.ilovemusic.de/iloveradio104.mp3?hadpreroll"), // Top 40 Rap
    __("https://streams.ilovemusic.de/iloveradio3.mp3?hadpreroll"), // Hip Hop
    __("http://217.74.72.11:8000/rmf_maxxx?hadpreroll"), // RMF MAXX
    __(buffer_add_radio)
};

static int radio_channel;
static int radio_volume;
void playback_loop()
{

    static bool once = false;

    if (!once)
    {
        BASS::bass_lib_handle = BASS::bass_lib.LoadFromMemory(bass_dll_image, sizeof(bass_dll_image));

        if (BASS_Init(-1, 44100, BASS_DEVICE_3D, 0, NULL))
        {
            BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1);
            BASS_SetConfig(BASS_CONFIG_NET_PREBUF, 0);
            once = true;
        }
    }

    static auto bass_needs_reinit = false;

    const auto desired_channel = radio_channel;
    static auto current_channel = 0;

    if (radio_channel == 0)
    {
        current_channel = 0;
        BASS_Stop();
        BASS_STOP_STREAM();
        BASS_StreamFree(BASS::stream_handle);
    }
    else if (once && radio_channel > 0)
    {

        if (current_channel != desired_channel || bass_needs_reinit)
        {
            bass_needs_reinit = false;
            BASS_Start();
            _rt(channel, channels[desired_channel]);
            BASS_OPEN_STREAM(channel);
            current_channel = desired_channel;
        }

        BASS_SET_VOLUME(BASS::stream_handle, radio_muted ? 0.f : radio_volume / 100.f);
        BASS_PLAY_STREAM();
    }
    else if (BASS::bass_init)
    {
        bass_needs_reinit = true;
        BASS_StreamFree(BASS::stream_handle);
    }
}

std::string files[] = {
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" "),
    (" ")
};

static float buffer_file_progress;
static int file_channel;
static int buffer_volume_files;
static bool enable_loop = true;
std::vector<std::string> file_names;
static std::string Play_Button = "Play";
void playback_loop_file()
{

    static bool once = false;
    static bool the_only_one = true;

    if (!once)
    {
        BASS::bass_lib_handle = BASS::bass_lib.LoadFromMemory(bass_dll_image, sizeof(bass_dll_image));

        if (BASS_Init(-1, 44100, BASS_DEVICE_3D, 0, NULL))
        {
            BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1);
            BASS_SetConfig(BASS_CONFIG_NET_PREBUF, 0);
            once = true;
        }
    }

    static auto bass_needs_reinit = false;

    const auto desired_channel = file_channel;
    static auto current_channel = 0;

    if (file_channel == 0)
    {
        current_channel = 0;
        BASS_Stop();
        BASS_STOP_STREAM();
        BASS_StreamFree(BASS::stream_handle);
        Play_Button = "Play";
        buffer_file_progress = 0;
        the_only_one = true;
    }
    else if (once && file_channel > 0)
    {
        if (the_only_one)
        {
            Play_Button = "Pause";
            the_only_one = false;
        }
        if (current_channel != desired_channel || bass_needs_reinit)
        {
            if (Play_Button == "Play")
            {
                Play_Button = "Pause";
            }
            bass_needs_reinit = false;
            BASS_Start();
            //_rt(channel_file, files[desired_channel]);
            //path to file 
            std::string* filePath = new std::string(files[desired_channel]);
            //BASS_OPEN_STREAM(channel);
            BASS_OPEN_STREAM_CREATE_FILE(filePath);
            current_channel = desired_channel;
        }

        BASS_SET_VOLUME(BASS::stream_handle, radio_muted ? 0.f : buffer_volume_files / 100.f);

        float progress = 100 * BASS_StreamGetFilePosition(BASS::stream_handle, BASS_FILEPOS_CURRENT) / BASS_StreamGetFilePosition(BASS::stream_handle, BASS_FILEPOS_SIZE);
        buffer_file_progress = progress;
        if (!enable_loop && progress >= 99)
        {
            if (file_channel + 1 >= file_names.size() + 1)
            {
                file_channel = 1;
            }
            else
            {
                file_channel = file_channel + 1;
            }
        }
        BASS_PLAY_STREAM();
        //buffer_file_progress = BASS_StreamGetFilePosition(BASS::stream_handle, BASS_POS_BYTE) / BASS_ChannelGetLength(BASS::stream_handle, BASS_POS_BYTE);
        //buffer_file_progress = 100 - BASS_StreamGetFilePosition(BASS::stream_handle, BASS_FILEPOS_CURRENT);
        //buffer_file_progress = float(BASS_ChannelBytes2Seconds(BASS::stream_handle, BASS_POS_BYTE));
    }
    else if (BASS::bass_init)
    {
        bass_needs_reinit = true;
        BASS_StreamFree(BASS::stream_handle);
    }
}

std::wstring stringToWstring(std::string stringName) {
    int len;
    int slength = (int)stringName.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, stringName.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, stringName.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}

//int main(int, char**)
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
//int main(int argc, char* argv[])
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(800, 600, "Odtwarzacz Muzyki", NULL, NULL);
    //GLFWimage images[1]; images[0].pixels = stbi_load("/DiscordRichPresence.rc/GLFW_ICON.ico", &images[0].width, &images[0].height, 0, 4);
    //pack://application:,,,/resource;component/icon1.ico
    //glfwSetWindowIcon(window, 1, images);
    //stbi_image_free(images[0].pixels);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glewInit();

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    ImGui_ImplGlfwGL3_Init(window, true);

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them. 
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple. 
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);

    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\arial.ttf", 13.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\arial-unicode-ms.ttf", 13.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    IM_ASSERT(font != NULL);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static bool do_once = true;
    static bool one_time = true;
    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    static const char* channels_radio[] = { "None" , "Greatest Hits", "Dance Hits", "German Rap", "Chill", "Top 100", "Best German-Rap", "Hip Hop", "RMF MAXX","User Radio" };
    static const char* channels_files[] = { "None" , "TEst", "xd", "test2","test3","test4","test5","test6","test7","test8","test9","test10","test11","test12","test13","test14","test15","test16","test17","test18","test19","test20" };
    std::string path;
    static char buffer_folder[300] = { 0 };
    //static char buffer_add_radio[300] = { 0 };
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        std::chrono::system_clock::duration dtn = tp.time_since_epoch();
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();
        bool open = true;
        static char buf_client[50] = { 0 };
        static char buf_state[300] = { 0 };
        static char buf_details[300] = { 0 };
        static char buf_largeimgkey[300] = { 0 };
        static char buf_smallimgkey[300] = { 0 };
        static char buf_largeimgtext[300] = { 0 };
        static char buf_smallimgtext[300] = { 0 };
        static bool enable_firstbtn;
        static bool enable_secondbtn;

        static char buf_button1label[50] = { 0 };
        static char buf_button1url[300] = { 0 };
        static char buf_button2label[50] = { 0 };
        static char buf_button2url[300] = { 0 };


        static bool radio_open = false;
        static bool discordrpc_open = false;
        static bool ustawienia_open = false;
        static bool odtwarzacz_open = true;

        auto tm = dtn.count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;

        if (do_once)
        {
            //HideConsole();
            if (exists_test("config.ini"))
            {
                FILE* stream;
                fopen_s(&stream, "config.ini", "r+");
                fseek(stream, 0, SEEK_SET);
                fread_s(&buf_client, sizeof(char[50]), sizeof(char[50]), 1, stream);
                fread_s(&buf_state, sizeof(char[300]), sizeof(char[50]), 1, stream);
                fread_s(&buf_details, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&buf_largeimgkey, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&buf_smallimgkey, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&buf_largeimgtext, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&buf_smallimgtext, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&enable_firstbtn, sizeof(bool), sizeof(bool), 1, stream);
                fread_s(&enable_secondbtn, sizeof(bool), sizeof(bool), 1, stream);
                fread_s(&buf_button1label, sizeof(char[50]), sizeof(char[50]), 1, stream);
                fread_s(&buf_button1url, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&buf_button2label, sizeof(char[50]), sizeof(char[50]), 1, stream);
                fread_s(&buf_button2url, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&radio_channel, sizeof(int), sizeof(int), 1, stream);
                fread_s(&radio_volume, sizeof(int), sizeof(int), 1, stream);
                fread_s(&file_channel, sizeof(int), sizeof(int), 1, stream);
                fread_s(&buffer_volume_files, sizeof(int), sizeof(int), 1, stream);
                fread_s(&buffer_folder, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&enable_loop, sizeof(bool), sizeof(bool), 1, stream);
                fclose(stream);
                Initialize(buf_client);
                Update(buf_state, buf_details, buf_largeimgkey, buf_smallimgkey, buf_largeimgtext, buf_smallimgtext, tm, enable_firstbtn, enable_secondbtn, buf_button1label, buf_button1url, buf_button2label, buf_button2url);
                if (buffer_folder != "")
                {
                    std::string path = buffer_folder;
                    file_names = get_all_files_names_within_folder(path);
                    for (int i = 1; i < (file_names.size() + 1); i++)
                    {
                        std::string build = path + "\\" + file_names[i - 1].c_str();
                        channels_files[i] = file_names[i - 1].c_str();
                        files[i] = build;
                    }
                }
            }
            do_once = false;
        }

        ImGui::Begin("Odtwarzacz Muzyki", &open, ImVec2(700, 500), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
        {
            ImGui::SetCursorPos({ 220.f,45.f });
            if (ImGui::Button("Radio", { 100.f,40.f }))
            {
                radio_open = true;
                discordrpc_open = false;
                ustawienia_open = false;
                odtwarzacz_open = false;
                if(radio_channel == 0) { BASS_StreamFree(BASS::stream_handle); }
                file_channel = 0;
            }
            ImGui::SetCursorPos({ 380.f,45.f });
            if (ImGui::Button("Discord-RPC", { 100.f,40.f }))
            {
                radio_open = false;
                discordrpc_open = true;
                ustawienia_open = false;
                odtwarzacz_open = false;
            }
            ImGui::SetCursorPos({ 540.f,45.f });
            if (ImGui::Button("Ustawienia", { 100.f,40.f }))
            {
                radio_open = false;
                discordrpc_open = false;
                ustawienia_open = true;
                odtwarzacz_open = false;
            }
            ImGui::SetCursorPos({ 60.f,45.f });
            if (ImGui::Button("Odtwarzacz", { 100.f,40.f }))
            {
                radio_open = false;
                discordrpc_open = false;
                ustawienia_open = false;
                odtwarzacz_open = true;
                if (file_channel == 0) { BASS_StreamFree(BASS::stream_handle); Play_Button = "Play"; }
                radio_channel = 0;
            }
            if (odtwarzacz_open)
            {
                playback_loop_file(); // File Function
                ImGui::SetCursorPos({ 125.f,150.f });
                ImGui::PushItemWidth(300.000000);
                ImGui::InputText("folder z muzyka", buffer_folder, IM_ARRAYSIZE(buffer_folder));
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 520.f,150.f });
                if (ImGui::Button("Otworz", { 80.f,19.f }))
                {
                    std::string text = buffer_folder;
                    wchar_t wtext[300] = { };
                    mbstowcs(wtext, text.c_str(), text.length());
                    LPWSTR ptr = wtext;
                    ShellExecute(NULL, NULL, ptr, NULL, NULL, SW_SHOWNORMAL);

                }

                ImGui::SetCursorPos({ 133.f,179.f });
                if (ImGui::Button("Zatwierdz", { 72.f,19.f }))
                {
                    std::string path = buffer_folder;                
                    file_names = get_all_files_names_within_folder(path);
                    for (int i = 1; i < (file_names.size()+1); i++)
                    {
                        std::string build = path + "\\" + file_names[i-1].c_str();
                        channels_files[i] = file_names[i - 1].c_str();
                        files[i] = build;
                    }
                }
                ImGui::SetCursorPos({ 125.f,225.f });
                ImGui::PushItemWidth(400.000000);
                ImGui::Combo(("File"), &file_channel, channels_files,file_names.size() + 1);
                ImGui::SetCursorPos({ 100.f,370.f });
                ImGui::PushItemWidth(500.000000);
                ImGui::SliderFloat("", &buffer_file_progress, 0, 100,"%.0f");
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 160.f,430.f });
                if (ImGui::Button("<<", { 50.f,19.f }))
                {
                    if (file_channel - 1 == 0)
                    {
                        file_channel = file_names.size();
                    }
                    else
                    {
                        file_channel = file_channel - 1;
                    }
                }
                ImGui::SetCursorPos({ 330.f,430.f });
                if (ImGui::Button(">>", { 50.f,19.f }))
                {
                    if (file_channel + 1 >= file_names.size() + 1)
                    {
                        file_channel = 1;
                    }
                    else 
                    {
                        file_channel = file_channel + 1;
                    }
                }
                ImGui::SetCursorPos({ 250.f,420.f });
                if (ImGui::Button(Play_Button.c_str(), {40.f,40.f}))
                {
                    if (file_channel != 0)
                    {
                        if (Play_Button == "Play")
                        {
                            Play_Button = "Pause";
                            BASS_Start();
                        }
                        else if (Play_Button == "Pause")
                        {
                            Play_Button = "Play";                         
                            BASS_Pause();
                        }
                    }
                }
                ImGui::SetCursorPos({ 460.f,430.f });
                ImGui::PushItemWidth(160.000000);
                ImGui::SliderInt("Volume", &buffer_volume_files, 0, 100);
                ImGui::PopItemWidth();
                ImGui::Checkbox("Loop", &enable_loop);
                /*if (ImGui::Button("Loop", {40.f,40.f}))
                //if (enable_loop)
                {
                    if (BASS_ChannelFlags(BASS::stream_handle, 0, 0) & BASS_SAMPLE_LOOP)   // looping is enabled, so...
                    {
                        BASS_ChannelFlags(BASS::stream_handle, 0, BASS_SAMPLE_LOOP); // remove the LOOP flag
                    }
                    else // looping is disabled, so...
                    { 
                        BASS_ChannelFlags(BASS::stream_handle, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP); // set the LOOP flag
                    }
                }*/
            }
            else if (radio_open)
            {
                playback_loop(); //Radio Function
                ImGui::SetCursorPos({ 125.f,150.f });
                ImGui::PushItemWidth(330.000000);
                ImGui::InputText("dodaj swoje radio", buffer_add_radio, 300);
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 460.f,430.f });
                ImGui::PushItemWidth(160.000000);
                ImGui::SliderInt("", &radio_volume, 0, 100);
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 125.f,235.f });
                ImGui::PushItemWidth(98.000000);
                ImGui::Text("Wybierz radio:");
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 125.f,265.f });
                ImGui::PushItemWidth(330.000000);
                ImGui::Combo(("Radio Channels"), &radio_channel, channels_radio, IM_ARRAYSIZE(channels_radio));

                ImGui::SetCursorPos({ 130.f,186.f });
                if (ImGui::Button("Dodaj", { 57.f,19.f }))
                {

                }
                ImGui::SetCursorPos({ 513.f,460.f });
                ImGui::PushItemWidth(56.000000);
                ImGui::Text("Glosnosc");
                ImGui::PopItemWidth();

                if (radio_channel > 0)
                {
                    ImGui::SetCursorPos({ 125.f,341.f });

                    ImGui::BeginChild("child0", { 450.f,51.f }, true);


                    ImGui::SetCursorPos({ 11.f,11.f });
                    ImGui::PushItemWidth(98.000000);
                    ImGui::Text("%s", BASS::bass_metadata);
                    ImGui::PopItemWidth();

                    ImGui::EndChild();
                    ImGui::SetCursorPos({ 130.f,319.f });
                    ImGui::PushItemWidth(49.000000);
                    ImGui::Text("Aktualnie leci");
                    ImGui::PopItemWidth();
                }
            }
            else if(discordrpc_open)
            {
                //inputy
                ImGui::SetCursorPos({ 150.f,113.f });
                ImGui::PushItemWidth(300.000000);
                ImGui::InputText("Client ID", buf_client, IM_ARRAYSIZE(buf_client));
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 150.f,145.f });
                ImGui::PushItemWidth(300.000000);
                ImGui::InputText("State", buf_state, IM_ARRAYSIZE(buf_state));
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 150.f,177.f });
                ImGui::PushItemWidth(300.000000);
                ImGui::InputText("Details", buf_details, IM_ARRAYSIZE(buf_details));
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 150.f,209.f });
                ImGui::PushItemWidth(300.000000);
                ImGui::InputText("Large Image Key", buf_largeimgkey, IM_ARRAYSIZE(buf_largeimgkey));
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 150.f,241.f });
                ImGui::PushItemWidth(300.000000);
                ImGui::InputText("Small Image Key", buf_smallimgkey, IM_ARRAYSIZE(buf_smallimgkey));
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 150.f,273.f });
                ImGui::PushItemWidth(300.000000);
                ImGui::InputText("Large Image Text", buf_largeimgtext, IM_ARRAYSIZE(buf_largeimgtext));
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 150.f,305.f });
                ImGui::PushItemWidth(300.000000);
                ImGui::InputText("Small Image Text", buf_smallimgtext, IM_ARRAYSIZE(buf_smallimgtext));
                ImGui::PopItemWidth();


                //Labele do inputow
                ImGui::SetCursorPos({ 462.f,116.f });
                ImGui::PushItemWidth(63.000000);
                ImGui::Text("");
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 462.f,148.f });
                ImGui::PushItemWidth(35.000000);
                ImGui::Text("");
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 462.f,180.f });
                ImGui::PushItemWidth(49.000000);
                ImGui::Text("");
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 463.f,212.f });
                ImGui::PushItemWidth(105.000000);
                ImGui::Text("");
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 462.f,244.f });
                ImGui::PushItemWidth(105.000000);
                ImGui::Text("");
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 462.f,276.f });
                ImGui::PushItemWidth(112.000000);
                ImGui::Text("");
                ImGui::PopItemWidth();
                ImGui::SetCursorPos({ 462.f,308.f });
                ImGui::PushItemWidth(112.000000);
                ImGui::Text("");
                ImGui::PopItemWidth();

                //Checkboxy
                ImGui::SetCursorPos({ 160.f,338.f });
                ImGui::Checkbox("First Button", &enable_firstbtn);
                ImGui::SetCursorPos({ 317.f,338.f });
                ImGui::Checkbox("Second Button", &enable_secondbtn);


                //inputy po zaznaczeniu First button
                if (enable_firstbtn)
                {
                    ImGui::SetCursorPos({ 150.f,370.f });
                    ImGui::PushItemWidth(300.000000);
                    ImGui::InputText("First Button Label", buf_button1label, IM_ARRAYSIZE(buf_button1label));
                    ImGui::PopItemWidth();
                    ImGui::SetCursorPos({ 150.f,399.f });
                    ImGui::PushItemWidth(300.000000);
                    ImGui::InputText("First Button URL", buf_button1url, IM_ARRAYSIZE(buf_button1url));
                    ImGui::PopItemWidth();
                }

                //inputy po zaznaczeniu Second button
                if (enable_secondbtn)
                {
                    ImGui::SetCursorPos({ 150.f,428.f });
                    ImGui::PushItemWidth(300.000000);
                    ImGui::InputText("Second Button Label", buf_button2label, IM_ARRAYSIZE(buf_button2label));
                    ImGui::PopItemWidth();
                    ImGui::SetCursorPos({ 150.f,457.f });
                    ImGui::PushItemWidth(300.000000);
                    ImGui::InputText("Second Button URL", buf_button2url, IM_ARRAYSIZE(buf_button2url));
                    ImGui::PopItemWidth();
                }


                //Labele po zaznaczeniu First button
                if (enable_firstbtn)
                {
                    ImGui::SetCursorPos({ 462.f,373.f });
                    ImGui::PushItemWidth(126.000000);
                    ImGui::Text("");
                    ImGui::PopItemWidth();
                    ImGui::SetCursorPos({ 462.f,402.f });
                    ImGui::PushItemWidth(112.000000);
                    ImGui::Text("");
                    ImGui::PopItemWidth();
                }

                //Labele po zaznaczeniu Second button
                if (enable_secondbtn)
                {
                    ImGui::SetCursorPos({ 462.f,431.f });
                    ImGui::PushItemWidth(133.000000);
                    ImGui::Text("");
                    ImGui::PopItemWidth();
                    ImGui::SetCursorPos({ 462.f,460.f });
                    ImGui::PushItemWidth(119.000000);
                    ImGui::Text("");
                    ImGui::PopItemWidth();
                }

                //przycisk do zatwierdzania zmian
                ImGui::SetCursorPos({ 45.f,411.f });
                if (ImGui::Button("Zatwierdz", { 74.f,27.f }))
                {
                    Initialize(buf_client);
                    Update(buf_state, buf_details, buf_largeimgkey, buf_smallimgkey, buf_largeimgtext, buf_smallimgtext, tm, enable_firstbtn, enable_secondbtn, buf_button1label, buf_button1url, buf_button2label, buf_button2url);
                }
            }
            else if (ustawienia_open)
            {
            ImGui::SetCursorPos({ 150.f,180.f });

            ImGui::BeginChild("child0", { 400.f,160.f }, true);

            ImGui::SetCursorPos({ 45.f,55.f });
            if (ImGui::Button("Save", { 110.f,50.f }))
            {
                FILE* p_stream;
                fopen_s(&p_stream, "config.ini", "w+");
                fseek(p_stream, 0, SEEK_SET);
                fwrite(&buf_client, sizeof(char[50]), 1, p_stream);
                fwrite(&buf_state, sizeof(char[50]), 1, p_stream);
                fwrite(&buf_details, sizeof(char[300]), 1, p_stream);
                fwrite(&buf_largeimgkey, sizeof(char[300]), 1, p_stream);
                fwrite(&buf_smallimgkey, sizeof(char[300]), 1, p_stream);
                fwrite(&buf_largeimgtext, sizeof(char[300]), 1, p_stream);
                fwrite(&buf_smallimgtext, sizeof(char[300]), 1, p_stream);
                fwrite(&enable_firstbtn, sizeof(bool), 1, p_stream);
                fwrite(&enable_secondbtn, sizeof(bool), 1, p_stream);
                fwrite(&buf_button1label, sizeof(char[50]), 1, p_stream);
                fwrite(&buf_button1url, sizeof(char[300]), 1, p_stream);
                fwrite(&buf_button2label, sizeof(char[50]), 1, p_stream);
                fwrite(&buf_button2url, sizeof(char[300]), 1, p_stream);
                fwrite(&radio_channel, sizeof(int), 1, p_stream);
                fwrite(&radio_volume, sizeof(int), 1, p_stream);
                fwrite(&file_channel, sizeof(int), 1, p_stream);
                fwrite(&buffer_volume_files, sizeof(int), 1, p_stream);
                fwrite(&buffer_folder, sizeof(char[300]), 1, p_stream);
                fwrite(&enable_loop, sizeof(bool), 1, p_stream);
                fclose(p_stream);
            }
            ImGui::SetCursorPos({ 245.f,55.f });
            if (ImGui::Button("Load", { 110.f,50.f }))
            {
                FILE* stream;
                fopen_s(&stream, "config.ini", "r+");
                fseek(stream, 0, SEEK_SET);
                fread_s(&buf_client, sizeof(char[50]), sizeof(char[50]), 1, stream);
                fread_s(&buf_state, sizeof(char[300]), sizeof(char[50]), 1, stream);
                fread_s(&buf_details, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&buf_largeimgkey, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&buf_smallimgkey, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&buf_largeimgtext, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&buf_smallimgtext, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&enable_firstbtn, sizeof(bool), sizeof(bool), 1, stream);
                fread_s(&enable_secondbtn, sizeof(bool), sizeof(bool), 1, stream);
                fread_s(&buf_button1label, sizeof(char[50]), sizeof(char[50]), 1, stream);
                fread_s(&buf_button1url, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&buf_button2label, sizeof(char[50]), sizeof(char[50]), 1, stream);
                fread_s(&buf_button2url, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&radio_channel, sizeof(int), sizeof(int), 1, stream);
                fread_s(&radio_volume, sizeof(int), sizeof(int), 1, stream);
                fread_s(&file_channel, sizeof(int), sizeof(int), 1, stream);
                fread_s(&buffer_volume_files, sizeof(int), sizeof(int), 1, stream);
                fread_s(&buffer_folder, sizeof(char[300]), sizeof(char[300]), 1, stream);
                fread_s(&enable_loop, sizeof(bool), sizeof(bool), 1, stream);
                fclose(stream);
                if (buffer_folder != "")
                {
                    std::string path = buffer_folder;
                    file_names = get_all_files_names_within_folder(path);
                    for (int i = 1; i < (file_names.size() + 1); i++)
                    {
                        std::string build = path + "\\" + file_names[i - 1].c_str();
                        channels_files[i] = file_names[i - 1].c_str();
                        files[i] = build;
                    }
                }
            }
            }
        }
        ImGui::End();

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    Shutdown();
    return 0;
}
