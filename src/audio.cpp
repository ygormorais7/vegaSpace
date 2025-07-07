#include "../includes/audio.h"
#include <iostream>

// Ponteiros globais para os sons
static Mix_Music* g_music = nullptr;
static Mix_Chunk* g_sfx_shot = nullptr;

void audio_init() {
    // Inicializa o subsistema de áudio do SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Erro ao inicializar SDL: " << SDL_GetError() << std::endl;
        return;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Erro ao inicializar SDL_mixer: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return;
    }
    
    // Define o volume da música para 50% (0 a 128)
    Mix_VolumeMusic(64);
    
    std::cout << "Sistema de áudio inicializado com sucesso!" << std::endl;
}

void audio_cleanup() {
    // Libera os recursos de áudio
    if (g_sfx_shot) {
        Mix_FreeChunk(g_sfx_shot);
        g_sfx_shot = nullptr;
    }
    if (g_music) {
        Mix_FreeMusic(g_music);
        g_music = nullptr;
    }

    // Fecha o mixer e o SDL
    Mix_CloseAudio();
    SDL_Quit();
    std::cout << "Sistema de áudio finalizado." << std::endl;
}

void audio_load_music(const std::string& filepath) {
    g_music = Mix_LoadMUS(filepath.c_str());
    if (!g_music) {
        std::cerr << "Falha ao carregar música: " << filepath << " - Erro: " << Mix_GetError() << std::endl;
    } else {
        std::cout << "Música carregada: " << filepath << std::endl;
    }
}

void audio_load_sfx(const std::string& filepath) {
    g_sfx_shot = Mix_LoadWAV(filepath.c_str());
    if (!g_sfx_shot) {
        std::cerr << "Falha ao carregar efeito sonoro: " << filepath << " - Erro: " << Mix_GetError() << std::endl;
    } else {
        std::cout << "Efeito sonoro carregado: " << filepath << std::endl;
    }
}

void audio_play_music_loop() {
    if (g_music) {
        // Verifica se a música não está tocando para evitar reiniciar
        if (!Mix_PlayingMusic()) {
            // O argumento -1 faz a música tocar em loop infinito
            Mix_PlayMusic(g_music, -1);
        }
    }
}

void audio_stop_music() {
    // Para a música gradualmente
    Mix_FadeOutMusic(500);
}

void audio_play_sfx() {
    if (g_sfx_shot) {
        // Toca o efeito sonoro no primeiro canal disponível (-1)
        // O último argumento é o número de loops (0 = tocar uma vez)
        Mix_PlayChannel(-1, g_sfx_shot, 0);
    }
}