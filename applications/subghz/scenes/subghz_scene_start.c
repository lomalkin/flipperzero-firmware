#include "../subghz_i.h"

enum SubmenuIndex {
    SubmenuIndexRead = 10,
    SubmenuIndexSaved,
#ifdef LAB_TESTS
    SubmenuIndexTest,
#endif
    SubmenuIndexAddManualy,
    SubmenuIndexFrequencyAnalyzer,
    SubmenuIndexReadRAW,
};

void subghz_scene_start_submenu_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

void subghz_scene_start_on_enter(void* context) {
    SubGhz* subghz = context;
    if(subghz->state_notifications == SubGhzNotificationStateStarting) {
        subghz->state_notifications = SubGhzNotificationStateIDLE;
    }
    submenu_add_item(
        subghz->submenu, "Read", SubmenuIndexRead, subghz_scene_start_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu,
        "Read RAW",
        SubmenuIndexReadRAW,
        subghz_scene_start_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu, "Saved", SubmenuIndexSaved, subghz_scene_start_submenu_callback, subghz);
    submenu_add_item(
        subghz->submenu,
        "Add manually",
        SubmenuIndexAddManualy,
        subghz_scene_start_submenu_callback,
        subghz);
    submenu_add_item(
        subghz->submenu,
        "Frequency Analyzer",
        SubmenuIndexFrequencyAnalyzer,
        subghz_scene_start_submenu_callback,
        subghz);
#ifdef LAB_TESTS
    submenu_add_item(
        subghz->submenu, "Test", SubmenuIndexTest, subghz_scene_start_submenu_callback, subghz);
#endif
    submenu_set_selected_item(
        subghz->submenu, scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneStart));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewMenu);
}

bool subghz_scene_start_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexReadRAW) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexReadRAW);
            subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReadRAW);
            return true;
        } else if(event.event == SubmenuIndexRead) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexRead);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiver);
            return true;
        } else if(event.event == SubmenuIndexSaved) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexSaved);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaved);
            return true;
        } else if(event.event == SubmenuIndexAddManualy) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexAddManualy);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSetType);
            return true;
        } else if(event.event == SubmenuIndexFrequencyAnalyzer) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexFrequencyAnalyzer);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneFrequencyAnalyzer);
            return true;
#ifdef LAB_TESTS
        } else if(event.event == SubmenuIndexTest) {
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneStart, SubmenuIndexTest);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTest);
            return true;
#endif
        }
    }
    return false;
}

void subghz_scene_start_on_exit(void* context) {
    SubGhz* subghz = context;
    submenu_clean(subghz->submenu);
}
