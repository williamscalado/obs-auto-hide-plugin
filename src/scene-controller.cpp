#include "scene-controller.hpp"
#include <obs-module.h>
#include <obs-frontend-api.h>

SceneController::SceneController(QObject *parent) : QObject(parent) {}

SceneController::~SceneController() {}

void SceneController::set_action_delay(int ms) { action_delay_ms = ms; }

QStringList SceneController::get_available_scenes() {
  QStringList scenes;
  struct obs_frontend_source_list source_list = {};
  obs_frontend_get_scenes(&source_list);

  for (size_t i = 0; i < source_list.sources.num; i++) {
    obs_source_t *source = source_list.sources.array[i];
    const char *name = obs_source_get_name(source);
    scenes.append(QString::fromUtf8(name));
  }

  obs_frontend_source_list_free(&source_list);
  return scenes;
}

QStringList SceneController::get_scene_sources(const QString &scene_name) {
  QStringList sources;
  obs_source_t *scene_source =
      obs_get_source_by_name(scene_name.toUtf8().constData());

  if (!scene_source)
    return sources;

  obs_scene_t *scene = obs_scene_from_source(scene_source);
  if (scene) {
    obs_scene_enum_items(
        scene,
        [](obs_scene_t *, obs_sceneitem_t *item, void *param) {
          QStringList *list = static_cast<QStringList *>(param);
          obs_source_t *source = obs_sceneitem_get_source(item);
          const char *name = obs_source_get_name(source);
          list->append(QString::fromUtf8(name));
          return true;
        },
        &sources);
  }

  obs_source_release(scene_source);
  return sources;
}

void SceneController::save_current_state(const QStringList &source_names) {
  saved_states.clear();

  obs_source_t *current_scene_source = obs_frontend_get_current_scene();
  if (!current_scene_source)
    return;

  obs_scene_t *scene = obs_scene_from_source(current_scene_source);
  if (!scene) {
    obs_source_release(current_scene_source);
    return;
  }

  for (const QString &name : source_names) {
    obs_sceneitem_t *item =
        obs_scene_find_source(scene, name.toUtf8().constData());
    if (item) {
      SourceState state;
      state.name = name;
      state.was_visible = obs_sceneitem_visible(item);
      saved_states[name] = state;
    }
  }

  obs_source_release(current_scene_source);
}

void SceneController::hide_sources(const QStringList &source_names) {
  save_current_state(source_names);

  // Usar QTimer::singleShot para debouncing/delay
  QTimer::singleShot(action_delay_ms, [this, source_names]() {
    obs_source_t *current_scene_source = obs_frontend_get_current_scene();
    if (!current_scene_source)
      return;

    obs_scene_t *scene = obs_scene_from_source(current_scene_source);
    if (!scene) {
      obs_source_release(current_scene_source);
      return;
    }

    int count = 0;
    for (const QString &name : source_names) {
      obs_sceneitem_t *item =
          obs_scene_find_source(scene, name.toUtf8().constData());
      if (item && obs_sceneitem_visible(item)) {
        obs_sceneitem_set_visible(item, false);
        count++;
      }
    }

    obs_source_release(current_scene_source);

    if (count > 0) {
      blog(LOG_INFO, "[Auto Hide] Escondeu %d fontes", count);
    }
  });
}

void SceneController::restore_previous_state() {
  QTimer::singleShot(action_delay_ms, [this]() {
    obs_source_t *current_scene_source = obs_frontend_get_current_scene();
    if (!current_scene_source)
      return;

    obs_scene_t *scene = obs_scene_from_source(current_scene_source);
    if (!scene) {
      obs_source_release(current_scene_source);
      return;
    }

    int count = 0;
    for (const auto &[name, state] : saved_states) {
      // Só restaura se estava visível ANTES
      if (state.was_visible) {
        obs_sceneitem_t *item =
            obs_scene_find_source(scene, name.toUtf8().constData());
        if (item && !obs_sceneitem_visible(item)) {
          obs_sceneitem_set_visible(item, true);
          count++;
        }
      }
    }

    obs_source_release(current_scene_source);

    if (count > 0) {
      blog(LOG_INFO, "[Auto Hide] Restaurou %d fontes", count);
    }
  });
}

void SceneController::show_all_sources(const QStringList &source_names) {
  QTimer::singleShot(action_delay_ms, [this, source_names]() {
    obs_source_t *current_scene_source = obs_frontend_get_current_scene();
    if (!current_scene_source)
      return;

    obs_scene_t *scene = obs_scene_from_source(current_scene_source);
    if (!scene) {
      obs_source_release(current_scene_source);
      return;
    }

    for (const QString &name : source_names) {
      obs_sceneitem_t *item =
          obs_scene_find_source(scene, name.toUtf8().constData());
      if (item) {
        obs_sceneitem_set_visible(item, true);
      }
    }

    obs_source_release(current_scene_source);
  });
}

bool SceneController::is_source_visible(const QString &source_name) {
  obs_source_t *current_scene_source = obs_frontend_get_current_scene();
  if (!current_scene_source)
    return false;

  obs_scene_t *scene = obs_scene_from_source(current_scene_source);
  bool visible = false;

  if (scene) {
    obs_sceneitem_t *item =
        obs_scene_find_source(scene, source_name.toUtf8().constData());
    if (item) {
      visible = obs_sceneitem_visible(item);
    }
  }

  obs_source_release(current_scene_source);
  return visible;
}

void SceneController::set_source_visibility(const QString &source_name,
                                            bool visible) {
  obs_source_t *current_scene_source = obs_frontend_get_current_scene();
  if (!current_scene_source)
    return;

  obs_scene_t *scene = obs_scene_from_source(current_scene_source);
  if (scene) {
    obs_sceneitem_t *item =
        obs_scene_find_source(scene, source_name.toUtf8().constData());
    if (item) {
      obs_sceneitem_set_visible(item, visible);
    }
  }

  obs_source_release(current_scene_source);
}
