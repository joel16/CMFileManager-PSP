#include "config.h"
#include "fs.h"
#include "log.h"
#include "rapidjson/document.h"
#include "utils.h"

constexpr int config_version = 4;
constexpr char config_file[] = "{\n\t\"config_ver\": %d,\n\t\"sort\": %d,\n\t\"dark_theme\": %d,\n\t\"dev_options\": %d\n}\n";
static int config_version_holder = 0;
config_t cfg;

namespace Config {
    int Save(config_t config) {
        int ret = 0;
        char *buf = new char[128];
        int len = std::snprintf(buf, 128, config_file, config_version, cfg.sort, cfg.dark_theme, cfg.dev_options);
        
        if (R_FAILED(ret = FS::WriteFile("config.json", buf, len))) {
            Log::Error("Read config failed in Config_Save 0x%08x\n", ret);
            delete[] buf;
            return ret;
        }
        
        delete[] buf;
        return 0;
    }

    static void SetDefault(config_t &config) {
        config.sort = 0;
        config.dark_theme = false;
        config.dev_options = false;
    }

    int Load(void) {
        int ret = 0;
        
        // Set root path and current working directory based on model.
        cfg.cwd = is_psp_go? "ef0:" : "ms0:";
        
        if (is_psp_go)
            device = BROWSE_STATE_INTERNAL;
        else
            device = BROWSE_STATE_EXTERNAL;
        
        if (!FS::FileExists("config.json")) {
            Config::SetDefault(cfg);
            return Save(cfg);
        }
        
        u64 size = FS::GetFileSize("config.json");
        char *buf = new char[size];
        
        if (R_FAILED(ret = FS::ReadFile("config.json", buf, size))) {
            Log::Error("Read config failed in Config_Load 0x%08x\n", ret);
            delete[] buf;
            return ret;
        }
        
        buf[size] = '\0';
        
        rapidjson::Document document;
        document.Parse(buf);
        assert(document.IsObject());
        assert(document.HasMember("config_ver"));
        assert(document.HasMember("sort"));
        assert(document.HasMember("dark_theme"));
        assert(document.HasMember("dev_options"));
        
        config_version_holder = document["config_ver"].GetInt();
        cfg.sort = document["sort"].GetInt();
        cfg.dark_theme = document["dark_theme"].GetInt();
        cfg.dev_options = document["dev_options"].GetInt();
        
        delete[] buf;
        
        // delete[] config file if config file is updated. This will rarely happen.
        if (config_version_holder < config_version) {
            sceIoRemove("config.json");
            Config::SetDefault(cfg);
            return Config::Save(cfg);
        }
        
        return 0;
    }
}
