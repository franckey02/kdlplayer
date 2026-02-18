#include <raylib.h>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <atomic>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <random>
#include "libs/discord_rpc.h"

namespace fs = std::filesystem;

struct Song { std::string path, name; };

// --- PATH AND DESKTOP MANAGEMENT ---
struct PathManager {
    static std::string GetHome() {
        const char* home = getenv("HOME");
        return home ? std::string(home) : ".";
    }

    static void SetupDesktopEntry(const char* exePath) {
        fs::path pExe(exePath);
        std::string home = GetHome();
        std::string iconDir = home + "/.local/share/applications/icon";
        std::string iconPath = iconDir + "/icon.png";
        
        try {
            fs::create_directories(iconDir);
            std::string srcIcon = pExe.parent_path().string() + "/icon/icon.png";
            if (fs::exists(srcIcon)) {
                fs::copy_file(srcIcon, iconPath, fs::copy_options::overwrite_existing);
            }
        } catch (...) {}

        std::ofstream df(home + "/.local/share/applications/kdlplayer.desktop");
        if (df.is_open()) {
            df << "[Desktop Entry]\nType=Application\nName=kdlplayer\nExec=" << exePath 
               << "\nIcon=" << iconPath << "\nTerminal=false\nCategories=AudioVideo;Player;\n";
            df.close();
        }
    }
};

// --- PLAYER LOGIC ---
class Reproductor {
private:
    std::mt19937 rng{std::random_device{}()};
public:
    std::vector<Song> playlist;
    int currentIndex = 0;
    Music currentMusic = { 0 };
    std::atomic<bool> isLoaded{false}, isPaused{false}, quit{false};
    bool isShuffle = false;
    bool isLoop = false;
    float volume = 0.5f;

    void play() {
        if (isLoaded) {
            UnloadMusicStream(currentMusic);
            isLoaded = false;
        }
        
        if (!playlist.empty() && currentIndex >= 0 && currentIndex < (int)playlist.size()) {
            currentMusic = LoadMusicStream(playlist[currentIndex].path.c_str());
            if (currentMusic.stream.buffer != nullptr) {
                PlayMusicStream(currentMusic);
                SetMusicVolume(currentMusic, volume);
                isLoaded = true;
                isPaused = false;
            }
        }
    }

    void loadPlaylist(const std::string& folder) {
        if (folder.empty() || !fs::exists(folder)) return;
        
        std::vector<Song> temp;
        for (const auto& entry : fs::directory_iterator(folder)) {
            auto ext = entry.path().extension().string();
            if (ext == ".mp3" || ext == ".flac" || ext == ".ogg") 
                temp.push_back({entry.path().string(), entry.path().filename().string()});
        }

        if (!temp.empty()) {
            std::sort(temp.begin(), temp.end(), [](const auto& a, const auto& b) { return a.name < b.name; });
            playlist = std::move(temp);
            currentIndex = 0;
            play();
        }
    }

    void siguiente() {
        if (playlist.empty()) return;
        if (isLoop) {
            play();
        } else if (isShuffle && playlist.size() > 1) {
            int next;
            do { next = rng() % playlist.size(); } while (next == currentIndex);
            currentIndex = next;
            play();
        } else {
            currentIndex = (currentIndex + 1) % playlist.size();
            play();
        }
    }

    void anterior() {
        if (playlist.empty()) return;
        currentIndex = (currentIndex - 1 + (int)playlist.size()) % playlist.size();
        play();
    }
};

void AudioThread(Reproductor* p) {
    while (!p->quit) {
        if (p->isLoaded && !p->isPaused) UpdateMusicStream(p->currentMusic);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

std::string PickFolderDialog() {
    char path[1024];
    FILE *f = popen("zenity --file-selection --directory --title='kdlplayer - Select Music Folder'", "r");
    if (!f) return "";
    if (fgets(path, 1024, f)) {
        std::string s(path);
        if (!s.empty() && s.back() == '\n') s.pop_back();
        pclose(f);
        return s;
    }
    pclose(f);
    return "";
}

// --- MAIN ---
int main(int argc, char* argv[]) {
    char fullPath[1024];
    if (realpath(argv[0], fullPath)) PathManager::SetupDesktopEntry(fullPath);

    InitWindow(550, 280, "kdlplayer");
    InitAudioDevice();

    // Loading custom font from icon/font.ttf
    Font font = LoadFontEx("icon/font.ttf", 32, 0, 0); 
    if (font.texture.id == 0) font = GetFontDefault();
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    Reproductor player;
    player.loadPlaylist(PathManager::GetHome() + "/Music");

    std::thread audioUpdater(AudioThread, &player);
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // --- INPUT LOGIC ---
        if (IsKeyPressed(KEY_O)) {
            std::string folder = PickFolderDialog();
            if (!folder.empty()) player.loadPlaylist(folder);
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            if (m.y >= 170 && m.y <= 190 && m.x >= 25 && m.x <= 525) {
                float clickPos = (m.x - 25.0f) / 500.0f;
                SeekMusicStream(player.currentMusic, clickPos * GetMusicTimeLength(player.currentMusic));
            } 
            else if (m.y > 60) {
                player.isPaused = !player.isPaused;
                if (player.isPaused) PauseMusicStream(player.currentMusic); 
                else ResumeMusicStream(player.currentMusic);
            }
        }

        player.volume = std::clamp(player.volume + GetMouseWheelMove() * 0.05f, 0.0f, 1.0f);
        if (player.isLoaded) SetMusicVolume(player.currentMusic, player.volume);

        if (IsKeyPressed(KEY_RIGHT)) player.siguiente();
        if (IsKeyPressed(KEY_LEFT)) player.anterior();
        if (IsKeyPressed(KEY_R)) player.isShuffle = !player.isShuffle;
        if (IsKeyPressed(KEY_L)) player.isLoop = !player.isLoop;

        if (player.isLoaded && !player.isPaused) {
            if (GetMusicTimePlayed(player.currentMusic) >= GetMusicTimeLength(player.currentMusic) - 0.1f) {
                player.siguiente();
            }
        }

        // --- DRAWING ---
        BeginDrawing();
            ClearBackground({22, 22, 22, 255});
            
            // Header
            DrawRectangle(0, 0, 550, 60, {32, 32, 32, 255});
            DrawTextEx(font, "kdlplayer", {20, 15}, 32, 1, SKYBLUE);
            DrawTextEx(font, "By Ars Byte :b", {430, 25}, 14, 1, DARKGRAY);

            if (player.isLoaded) {
                // Large song title
                DrawTextEx(font, player.playlist[player.currentIndex].name.c_str(), {25, 90}, 28, 1, RAYWHITE);
                
                // Progress Bar
                float prog = GetMusicTimePlayed(player.currentMusic) / GetMusicTimeLength(player.currentMusic);
                DrawRectangle(25, 175, 500, 6, {50, 50, 50, 255});
                DrawRectangle(25, 175, (int)(500 * prog), 6, SKYBLUE);
                DrawCircle(25 + (int)(500 * prog), 178, 7, RAYWHITE);
            } else {
                DrawTextEx(font, "No music loaded. Press 'O' to open folder.", {25, 90}, 20, 1, GRAY);
            }
            
            // Footer UI with Arrow controls explanation
            DrawTextEx(font, "SHUFFLE", {380, 230}, 16, 1, player.isShuffle ? SKYBLUE : GRAY);
            DrawTextEx(font, "LOOP", {480, 230}, 16, 1, player.isLoop ? SKYBLUE : GRAY);
            
            // Helpful control labels
            DrawTextEx(font, "O: OPEN | ARROWS: PREV/NEXT | R: SHUFFLE | L: LOOP", {25, 255}, 13, 1, GRAY);
            DrawTextEx(font, "CLICK: PAUSE/RESUME | WHEEL: VOLUME", {25, 235}, 13, 1, GRAY);
            
        EndDrawing();
    }

    // --- CLEANUP ---
    player.quit = true;
    if (audioUpdater.joinable()) audioUpdater.join();
    if (player.isLoaded) UnloadMusicStream(player.currentMusic);
    UnloadFont(font);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
