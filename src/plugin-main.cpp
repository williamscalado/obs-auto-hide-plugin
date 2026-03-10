#include "auto-hide-dock.hpp"
#include "holyrics-client.hpp"
#include "propresent-client.hpp"
#include "plugin-config.hpp"
#include "scene-controller.hpp"
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/bmem.h>

OBS_DECLARE_MODULE()
// OBS_MODULE_USE_DEFAULT_LOCALE("auto-hide-scenes", "en-US")

class AutoHidePlugin {
private:
  PluginConfig config;
  IPresentationClient *active_client = nullptr;
  SceneController *scene_controller;
  AutoHideDockWidget *dock_widget;

  void setup_client() {
    // Deleta o anterior se existir
    if (active_client) {
        active_client->disconnect();
        delete active_client;
        active_client = nullptr;
    }

    // Instancia o novo baseado na config
    if (config.client_type == "ProPresent") {
        active_client = new ProPresentClient();
        blog(LOG_INFO, "[Auto Hide] Inicializando ProPresent Client");
    } else {
        active_client = new HolyricsClient();
        blog(LOG_INFO, "[Auto Hide] Inicializando Holyrics Client");
    }

    // Configurar callbacks
    active_client->on_verse_changed = [this](bool visible) {
      on_verse_state_changed(visible);
    };

    active_client->on_deactivation_requested = [this]() {
        if (dock_widget) {
            scene_controller->hide_sources(config.sources_to_hide);
            dock_widget->set_active(false, false);
            blog(LOG_INFO, "[Auto Hide] Desativação automática (MUSIC): Fontes ocultadas e plugin parado.");
        }
    };
  }

public:
  AutoHidePlugin() {
    scene_controller = new SceneController();
    
    // O DockWidget precisa do PONTEIRO para onde armazenamos o cliente ativo, 
    // pois poderemos recriar a instância do cliente abaixo
    dock_widget =
        new AutoHideDockWidget(config, &active_client, scene_controller);
  }

  ~AutoHidePlugin() {
    if (active_client) delete active_client;
    delete scene_controller;
    // dock_widget é deletado pelo OBS ao fechar ou remover dock
  }

  void on_verse_state_changed(bool verse_visible) {
    blog(LOG_INFO, "[Auto Hide] Versículo: %s",
         verse_visible ? "APARECEU" : "SUMIU");

    if (verse_visible) {
      // Esconder fontes configuradas
      scene_controller->hide_sources(config.sources_to_hide);
    } else {
      // Restaurar estado anterior
      if (config.restore_previous_state) {
        scene_controller->restore_previous_state();
      } else {
        scene_controller->show_all_sources(config.sources_to_hide);
      }
    }

    // Atualizar UI
    dock_widget->update_last_event(verse_visible);
  }

  void load_config() {
    char *path_ptr = obs_module_config_path("config.json");
    QString old_client_type = config.client_type;
    
    if (path_ptr) {
        QString path = QString(path_ptr);
        blog(LOG_INFO, "[Auto Hide] Carregando configuração de: %s", path_ptr);
        config.load_from_file(path);
        bfree(path_ptr);
    } else {
        blog(LOG_WARNING, "[Auto Hide] obs_module_config_path retornou NULL");
    }

    // Se mudou o client_type, ou se é a primeira vez (active_client nulo), configura o cliente
    if (!active_client || old_client_type != config.client_type) {
        setup_client();
    }

    // Se é Holyrics, ele suporta as configurações estendidas (podemos testar com dynamic_cast pra ser seguros)
    HolyricsClient* hc = dynamic_cast<HolyricsClient*>(active_client);
    if (hc) {
        hc->set_polling_interval(config.polling_interval_ms);
        hc->set_disable_in_music(config.disable_in_music);
    } else {
        ProPresentClient* ppc = dynamic_cast<ProPresentClient*>(active_client);
        if (ppc) {
             ppc->set_polling_interval(config.polling_interval_ms);
             ppc->set_disable_in_music(config.disable_in_music);
        }
    }

    scene_controller->set_action_delay(config.action_delay_ms);
    scene_controller->set_auto_transition(config.auto_transition);

    // Se ativado automaticamente
    if (config.auto_activate) {
      dock_widget->set_active(true);
    }
  }

  void save_config() {
    char *path_ptr = obs_module_config_path("config.json");
    if (path_ptr) {
        QString path = QString(path_ptr);
        config.save_to_file(path);
        bfree(path_ptr);
    }
  }

  AutoHideDockWidget *get_dock_widget() { return dock_widget; }
};

// Instância global
static AutoHidePlugin *plugin_instance = nullptr;

bool obs_module_load(void) {
  blog(LOG_INFO, "[Auto Hide] Plugin carregado v1.0.0");

  plugin_instance = new AutoHidePlugin();

  // Registrar Dock
  obs_frontend_add_dock_by_id("auto-hide-scenes", "Cidade Viva Plugin", plugin_instance->get_dock_widget());

  // Carregar configuração
  plugin_instance->load_config();

  return true;
}

void obs_module_unload(void) {
  blog(LOG_INFO, "[Auto Hide] Plugin descarregado");

  if (plugin_instance) {
    plugin_instance->save_config();
    delete plugin_instance;
    plugin_instance = nullptr;
  }
}

const char *obs_module_name(void) { return "Auto Hide Scenes"; }

const char *obs_module_description(void) {
  return "Esconde automaticamente fontes quando versículos bíblicos aparecem "
         "no Holyrics";
}
