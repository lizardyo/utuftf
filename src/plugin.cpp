#include <iostream>
#include <unordered_map>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <filesystem>
#include <windows.h>

struct RemotePlayerAppearance {
    uint32_t playerID;
    const char* playerProfileName;
    const char* equippedTopShirtName;    
    const char* equippedBottomPantsName; 
    const char* equippedShoesName;       
    void* nativeVisualInstancePointer;
};

struct DynamicStreamAsset {
    std::string assetStringIdentifier;
    std::string temporaryCachedPath;
    bool isCommittedToVRAM = false;
};

std::unordered_map<std::string, DynamicStreamAsset> g_StreamRegistry;
std::queue<std::string> g_AssetDownloadQueue;
std::mutex g_StreamMutex;
bool g_PipelineWorkerActive = true;

const std::string CACHE_ROOT_DIR = "./ReSkate/Cache/SkinSync/";

void StreamAssetDownloadWorkerPipeline() {
    while (g_PipelineWorkerActive) {
        std::string activeAssetID = "";
        {
            std::lock_guard<std::mutex> lock(g_StreamMutex);
            if (!g_AssetDownloadQueue.empty()) {
                activeAssetID = g_AssetDownloadQueue.front();
                g_AssetDownloadQueue.pop();
            }
        }

        if (!activeAssetID.empty()) {
            std::string localTargetFile = CACHE_ROOT_DIR + activeAssetID + ".cas";
            if (!std::filesystem::exists(localTargetFile)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
            }
            std::lock_guard<std::mutex> lock(g_StreamMutex);
            g_StreamRegistry[activeAssetID].temporaryCachedPath = localTargetFile;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void PurgeLocalSkinCache() {
    if (std::filesystem::exists(CACHE_ROOT_DIR)) {
        std::error_code err;
        std::filesystem::remove_all(CACHE_ROOT_DIR, err);
    }
}

extern "C" __declspec(dllexport) const char* GetPluginName() {
    return "MEMORY OPTIMIZER: SKIN SYNC";
}

extern "C" __declspec(dllexport) void OnPluginMenuRender() {
}

extern "C" __declspec(dllexport) void OnUpdateTick(void* clientPlayerManagerStructure) {
    if (!clientPlayerManagerStructure) return;

    RemotePlayerAppearance* serverLobbyPlayers = *(RemotePlayerAppearance**)((uintptr_t)clientPlayerManagerStructure + 0x48);
    int connectedPeerCount = *(int*)((uintptr_t)clientPlayerManagerStructure + 0x80);

    for (int i = 0; i < connectedPeerCount; i++) {
        RemotePlayerAppearance& targetPeer = serverLobbyPlayers[i];
        if (!targetPeer.equippedTopShirtName || strlen(targetPeer.equippedTopShirtName) == 0) continue;

        std::string currentTopID = std::string(targetPeer.equippedTopShirtName);
        std::lock_guard<std::mutex> lock(g_StreamMutex);

        if (g_StreamRegistry.find(currentTopID) == g_StreamRegistry.end()) {
            DynamicStreamAsset incomingAsset;
            incomingAsset.assetStringIdentifier = currentTopID;
            g_StreamRegistry[currentTopID] = incomingAsset;
            g_AssetDownloadQueue.push(currentTopID); 
        } 
        else if (!g_StreamRegistry[currentTopID].temporaryCachedPath.empty() && !g_StreamRegistry[currentTopID].isCommittedToVRAM) {
            uintptr_t meshInstanceRenderer = *(uintptr_t*)((uintptr_t)targetPeer.nativeVisualInstancePointer + 0x18);
            if (meshInstanceRenderer) {
                *(const char**)(meshInstanceRenderer + 0x30) = g_StreamRegistry[currentTopID].temporaryCachedPath.c_str();
                g_StreamRegistry[currentTopID].isCommittedToVRAM = true;
            }
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        std::filesystem::create_directories(CACHE_ROOT_DIR);
        std::thread(StreamAssetDownloadWorkerPipeline).detach();
        break;
    case DLL_PROCESS_DETACH:
        g_PipelineWorkerActive = false;
        PurgeLocalSkinCache(); 
        break;
    }
    return TRUE;
}
