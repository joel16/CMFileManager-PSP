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
    static const char *path = "config.json";

    int Save(config_t config) {
        int ret = 0;
        char buf[128];
        int len = std::snprintf(buf, 128, config_file, config_version, cfg.sort, cfg.dark_theme, cfg.dev_options);
        
        if (R_FAILED(ret = FS::WriteFile(path, buf, len))) {
            Log::Error("%s failed: 0x%08x\n", __func__, ret);
            return ret;
        }
        
        return 0;
    }

    int Load(void) {
        int ret = 0;
        
        // Set root path and current working directory based on model.
        cfg.cwd = isPSPGo? "ef0:" : "ms0:";
        device = isPSPGo ? BROWSE_STATE_INTERNAL : BROWSE_STATE_EXTERNAL;
        
        if (!FS::FileExists(path)) {
            cfg = config_t();
            cfg.cwd = isPSPGo? "ef0:" : "ms0:";
            return Config::Save(cfg);
        }
        
        u64 size = FS::GetFileSize(path);
        char *buf = new char[size];
        
        if (R_FAILED(ret = FS::ReadFile(path, buf, size))) {
            Log::Error("%s(FS::ReadFile) failed: 0x%08x\n", __func__, ret);
            delete[] buf;
            return ret;
        }
        
        buf[size] = '\0';
        rapidjson::Document document;
        document.Parse(buf);
        
        if ((!document.IsObject()) || (!document.HasMember("config_ver")) || (!document.HasMember("sort")) ||
            (!document.HasMember("dark_theme")) || (!document.HasMember("dev_options"))) {
            Log::Error("%s failed: Malformed config file, resetting\n", __func__);
            cfg = config_t();
            cfg.cwd = isPSPGo? "ef0:" : "ms0:";
            return Config::Save(cfg);
        }
        
        config_version_holder = document["config_ver"].GetInt();
        cfg.sort = document["sort"].GetInt();
        cfg.dark_theme = document["dark_theme"].GetInt();
        cfg.dev_options = document["dev_options"].GetInt();
        
        delete[] buf;
        
        // delete[] config file if config file is updated. This will rarely happen.
        if (config_version_holder < config_version) {
            sceIoRemove(path);
            cfg = config_t();
            cfg.cwd = isPSPGo? "ef0:" : "ms0:";
            return Config::Save(cfg);
        }
        
        return 0;
    }
}
