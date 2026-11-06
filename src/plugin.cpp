-- RE-SKATE ULTRA-OPTIMIZED ON-DEMAND COSMETIC STREAMER
local SkinSync = {
    pluginName = "MEMORY OPTIMIZER: SKIN SYNC",
    cacheDir = "./ReSkate/Cache/SkinSync/",
    activeRegistry = {},
    downloadQueue = {}
}

-- Initialize clean disk workspace (Memory Optimization Rule)
function SkinSync.Init()
    -- Wipes previous temporary single-garment storage locations on boot
    os.execute("rmdir /s /q \"ReSkate\\Cache\\SkinSync\" 2>nul")
    os.execute("mkdir \"ReSkate\\Cache\\SkinSync\" 2>nul")
    print("[" .. SkinSync.pluginName .. "] Temp file system ready. On-demand sync armed.")
end

-- Intercepts active multiplayer session network data packets at 60Hz
function SkinSync.OnUpdateTick(playerManagerInstance)
    if not playerManagerInstance then return end

    -- Read current lobby connections directly from multiplayer peer vectors
    local connectedPeers = playerManagerInstance.GetRemotePlayers()
    if not connectedPeers then return end

    for _, player in ipairs(connectedPeers) do
        -- Intercept individual garment text paths (e.g., "Own_Carhartt_Loyde_Shirt_Black")
        local topShirtName = player.GetEquippedItemName("top_shirt")

        if topShirtName and topShirtName ~= "" and not topShirtName:find("Gen_Tshirt") then
            -- O(1) CPU Lookup efficiency check to ensure we don't repeat downloads
            if not SkinSync.activeRegistry[topShirtName] then
                SkinSync.activeRegistry[topShirtName] = {
                    identifier = topShirtName,
                    localPath = SkinSync.cacheDir .. topShirtName .. ".cas",
                    isLoaded = false
                }
                -- Queue ONLY this single garment file instead of downloading a whole 15GB pack
                table.insert(SkinSync.downloadQueue, topShirtName)
                SkinSync.StreamSingleAsset(topShirtName)
            else
                -- If asset file exists locally on your hard drive, force load it immediately
                local assetData = SkinSync.activeRegistry[topShirtName]
                if not assetData.isLoaded then
                    player.OverrideAppearanceMesh("top_shirt", assetData.localPath)
                    assetData.isLoaded = true
                end
            end
        end
    end
end

-- Highly optimized isolated background file fetcher
function SkinSync.StreamSingleAsset(assetName)
    -- Asynchronously streams only the needed file to lower CPU costs and avoid stutters
    local command = string.format("powershell -Command \"Start-Job -ScriptBlock { Invoke-WebRequest -Uri 'https://reskateskinsync.com' -OutFile '%s%s.cas' }\" >nul", 
        assetName, SkinSync.cacheDir, assetName)
    os.execute(command)
end

-- Bind directly to ReSkate Trainer Interface hooks
ReSkate.RegisterPlugin({
    name = SkinSync.pluginName,
    onLoad = SkinSync.Init,
    onTick = SkinSync.OnUpdateTick
})
