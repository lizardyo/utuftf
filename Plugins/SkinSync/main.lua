-- RE-SKATE ULTRA-OPTIMIZED ON-DEMAND COSMETIC STREAMER
local SkinSync = {
    pluginName = "MEMORY OPTIMIZER: SKIN SYNC",
    cacheDir = "./ReSkate/Cache/SkinSync/",
    activeRegistry = {}
}

-- Initialize clean disk workspace (Wipes temp items on boot to save memory)
function SkinSync.Init()
    os.execute("rmdir /s /q \"ReSkate\\Cache\\SkinSync\" 2>nul")
    os.execute("mkdir \"ReSkate\\Cache\\SkinSync\" 2>nul")
    print("[" .. SkinSync.pluginName .. "] Temp workspace ready. Stream active.")
end

-- Intercepts live server connection frames across multiplayer lobbies at 60Hz
function SkinSync.OnUpdateTick(playerManagerInstance)
    if not playerManagerInstance then return end

    -- Read current lobby connections directly from multiplayer peer vectors
    local connectedPeers = playerManagerInstance.GetRemotePlayers()
    if not connectedPeers then return end

    for _, player in ipairs(connectedPeers) do
        -- Intercept individual garment text paths before the engine fallback overrides them
        local topShirtName = player.GetEquippedItemName("top_shirt")

        if topShirtName and topShirtName ~= "" and not topShirtName:find("Gen_Tshirt") then
            -- O(1) CPU Efficiency optimization check to prevent duplicate requests
            if not SkinSync.activeRegistry[topShirtName] then
                SkinSync.activeRegistry[topShirtName] = {
                    identifier = topShirtName,
                    localPath = SkinSync.cacheDir .. topShirtName .. ".cas",
                    isLoaded = false
                }
                -- Silently download ONLY this individual garment asset via a fast background thread
                SkinSync.StreamSingleAsset(topShirtName)
            else
                -- If asset exists locally, hot-swap texture pipelines to render it immediately
                local assetData = SkinSync.activeRegistry[topShirtName]
                if not assetData.isLoaded and io.open(assetData.localPath, "r") ~= nil then
                    player.OverrideAppearanceMesh("top_shirt", assetData.localPath)
                    assetData.isLoaded = true
                end
            end
        end
    end
end

-- Highly optimized isolated background file fetcher
function SkinSync.StreamSingleAsset(assetName)
    local command = string.format("powershell -Command \"Start-Job -ScriptBlock { Invoke-WebRequest -Uri 'https://reskateskinsync.com' -OutFile '%s%s.cas' }\" >nul", 
        assetName, SkinSync.cacheDir, assetName)
    os.execute(command)
end

-- Bind directly to ReSkate Trainer Interface hooks
if ReSkate and ReSkate.RegisterPlugin then
    ReSkate.RegisterPlugin({
        name = SkinSync.pluginName,
        onLoad = SkinSync.Init,
        onTick = SkinSync.OnUpdateTick
    })
end
