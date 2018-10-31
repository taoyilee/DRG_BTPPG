#include "app_main.h"
#include "uib_app_manager.h"
#include <bluetooth.h>

#include <dlog.h>
#include <glib.h> /* For GList */
#define DRG_LOG_TAG "DRG"


/* app event callbacks */
static bool _on_create_cb(void *user_data);
static void _on_terminate_cb(void *user_data);
static void _on_app_control_cb(app_control_h app_control, void *user_data);
static void _on_resume_cb(void *user_data);
static void _on_pause_cb(void *user_data);
static void _on_low_memory_cb(app_event_info_h event_info, void *user_data);
static void _on_low_battery_cb(app_event_info_h event_info, void *user_data);
static void _on_device_orientation_cb(app_event_info_h event_info, void *user_data);
static void _on_language_changed_cb(app_event_info_h event_info, void *user_data);
static void _on_region_format_changed_cb(app_event_info_h event_info, void *user_data);

void
nf_hw_back_cb(void* param, Evas_Object * evas_obj, void* event_info) {
	//TODO : user define code
	evas_obj = uib_views_get_instance()->get_window_obj()->app_naviframe;
	elm_naviframe_item_pop(evas_obj);
}

void
win_del_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

Eina_Bool
nf_root_it_pop_cb(void* elm_win, Elm_Object_Item *it) {
	elm_win_lower(elm_win);
	return EINA_FALSE;
}

app_data *uib_app_create()
{
	return calloc(1, sizeof(app_data));
}

void uib_app_destroy(app_data *user_data)
{
	uib_app_manager_get_instance()->free_all_view_context();
	free(user_data);
}

int uib_app_run(app_data *user_data, int argc, char **argv)
{
	ui_app_lifecycle_callback_s cbs =
	{
		.create = _on_create_cb,
		.terminate = _on_terminate_cb,
		.pause = _on_pause_cb,
		.resume = _on_resume_cb,
		.app_control = _on_app_control_cb,
	};

	app_event_handler_h handlers[5] =
	{NULL, };

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, _on_low_battery_cb, user_data);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, _on_low_memory_cb, user_data);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, _on_device_orientation_cb, user_data);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, _on_language_changed_cb, user_data);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, _on_region_format_changed_cb, user_data);

	return ui_app_main(argc, argv, &cbs, user_data);
}

void
app_get_resource(const char *res_file_in, char *res_path_out, int res_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(res_path_out, res_path_max, "%s%s", res_path, res_file_in);
		free(res_path);
	}
}


void
adapter_device_discovery_state_changed_cb(int result, bt_adapter_device_discovery_state_e discovery_state,
                                          bt_adapter_device_discovery_info_s *discovery_info, void* user_data)
{
    if (result != BT_ERROR_NONE) {
        dlog_print(DLOG_ERROR, DRG_LOG_TAG, "[adapter_device_discovery_state_changed_cb] failed! result(%d).", result);

        return;
    }
    GList** searched_device_list = (GList**)user_data;
    switch (discovery_state) {
    case BT_ADAPTER_DEVICE_DISCOVERY_STARTED:
        dlog_print(DLOG_INFO, DRG_LOG_TAG, "BT_ADAPTER_DEVICE_DISCOVERY_STARTED");
        break;
    case BT_ADAPTER_DEVICE_DISCOVERY_FINISHED:
        dlog_print(DLOG_INFO, DRG_LOG_TAG, "BT_ADAPTER_DEVICE_DISCOVERY_FINISHED");
        break;
    case BT_ADAPTER_DEVICE_DISCOVERY_FOUND:
        dlog_print(DLOG_INFO, DRG_LOG_TAG, "BT_ADAPTER_DEVICE_DISCOVERY_FOUND");
        if (discovery_info != NULL) {
            dlog_print(DLOG_INFO, DRG_LOG_TAG, "Device Address: %s", discovery_info->remote_address);
            //dlog_print(DLOG_INFO, DRG_LOG_TAG, "Device Name is: %s", discovery_info->remote_name);
            bt_adapter_device_discovery_info_s * new_device_info = malloc(sizeof(bt_adapter_device_discovery_info_s));
            if (new_device_info != NULL) {
                memcpy(new_device_info, discovery_info, sizeof(bt_adapter_device_discovery_info_s));
                new_device_info->remote_address = strdup(discovery_info->remote_address);
                new_device_info->remote_name = strdup(discovery_info->remote_name);
                *searched_device_list = g_list_append(*searched_device_list, (gpointer)new_device_info);
            }
        }
        break;
    }
}

/* Server address for connecting */
char *bt_server_address = NULL;
const char *remote_server_name = "";

bool
adapter_bonded_device_cb(bt_device_info_s *device_info, void *user_data)
{
    if (device_info == NULL)
        return true;
    if (!strcmp(device_info->remote_name, (char*)user_data)) {
        dlog_print(DLOG_INFO, DRG_LOG_TAG, "The server device is found in bonded device list. address(%s)",
                   device_info->remote_address);
        bt_server_address = strdup(device_info->remote_address);
        /* If you want to stop iterating, you can return "false" */
    }
    /* Get information about bonded device */
    int count_of_bonded_device = 1;
    dlog_print(DLOG_INFO, DRG_LOG_TAG, "Get information about the bonded device(%d)", count_of_bonded_device);
    dlog_print(DLOG_INFO, DRG_LOG_TAG, "remote address = %s.", device_info->remote_address);
    dlog_print(DLOG_INFO, DRG_LOG_TAG, "remote name = %s.", device_info->remote_name);
    dlog_print(DLOG_INFO, DRG_LOG_TAG, "service count = %d.", device_info->service_count);
    dlog_print(DLOG_INFO, DRG_LOG_TAG, "bonded?? %d.", device_info->is_bonded);
    dlog_print(DLOG_INFO, DRG_LOG_TAG, "connected?? %d.", device_info->is_connected);
    dlog_print(DLOG_INFO, DRG_LOG_TAG, "authorized?? %d.", device_info->is_authorized);

    dlog_print(DLOG_INFO, DRG_LOG_TAG, "major_device_class %d.", device_info->bt_class.major_device_class);
    dlog_print(DLOG_INFO, DRG_LOG_TAG, "minor_device_class %d.", device_info->bt_class.minor_device_class);
    dlog_print(DLOG_INFO, DRG_LOG_TAG, "major_service_class_mask %d.", device_info->bt_class.major_service_class_mask);
    count_of_bonded_device++;

    /* Keep iterating */

    return true;
}


static bool _on_create_cb(void *user_data)
{
	/*
	 * This area will be auto-generated when you add or delete user_view
	 * Please do not hand edit this area. The code may be lost.
	 */
	uib_app_manager_st* app_manager = uib_app_manager_get_instance();

	app_manager->initialize();

	bt_error_e ret;

	ret = bt_initialize();
	if (ret != BT_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, DRG_LOG_TAG, "[bt_initialize] failed.");
	}else{
		dlog_print(DLOG_INFO, DRG_LOG_TAG, "[bt_initialize] succeeded.");
	}
	bt_adapter_state_e adapter_state;
	/* Check whether the Bluetooth adapter is enabled */
	ret = bt_adapter_get_state(&adapter_state);
	if (ret != BT_ERROR_NONE) {
	    dlog_print(DLOG_ERROR, DRG_LOG_TAG, "[bt_adapter_get_state] failed");
	}
	/* If the Bluetooth adapter is not enabled */
	if (adapter_state == BT_ADAPTER_DISABLED)
	    dlog_print(DLOG_ERROR, DRG_LOG_TAG, "Bluetooth adapter is not enabled. You should enable Bluetooth!!");


	if (adapter_state == BT_ADAPTER_ENABLED) {
		dlog_print(DLOG_INFO, DRG_LOG_TAG, "[adapter_state] Bluetooth is enabled!");

		/* Get information about Bluetooth adapter */
		char *local_address = NULL;
		bt_adapter_get_address(&local_address);
		dlog_print(DLOG_INFO, DRG_LOG_TAG, "[adapter_state] Adapter address: %s.", local_address);
		if (local_address)
			free(local_address);
		char *local_name;
		bt_adapter_get_name(&local_name);
		dlog_print(DLOG_INFO, DRG_LOG_TAG, "[adapter_state] Adapter name: %s.", local_name);
		if (local_name)
			free(local_name);
		/* Visibility mode of the Bluetooth device */
		bt_adapter_visibility_mode_e mode;
		/*
		   Duration until the visibility mode is changed
		   so that other devices cannot find your device
		*/
		int duration = 1;
		bt_adapter_get_visibility(&mode, &duration);
		switch (mode) {
		case BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE:
			dlog_print(DLOG_INFO, DRG_LOG_TAG,
					   "[adapter_state] Visibility: NON_DISCOVERABLE");
			break;
		case BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE:
			dlog_print(DLOG_INFO, DRG_LOG_TAG,
					   "[adapter_state] Visibility: GENERAL_DISCOVERABLE");
			break;
		case BT_ADAPTER_VISIBILITY_MODE_LIMITED_DISCOVERABLE:
			dlog_print(DLOG_INFO, DRG_LOG_TAG,
					   "[adapter_state] Visibility: LIMITED_DISCOVERABLE");
			break;
		}
	} else {
		dlog_print(DLOG_INFO, DRG_LOG_TAG, "[adapter_state] Bluetooth is disabled!");
		/*
		   When you try to get device information
		   by invoking bt_adapter_get_name(), bt_adapter_get_address(),
		   or bt_adapter_get_visibility(), BT_ERROR_NOT_ENABLED occurs
		*/
	}
//	GList *devices_list = NULL;
//	ret = bt_adapter_set_device_discovery_state_changed_cb(adapter_device_discovery_state_changed_cb,
//	                                                       (void*)&devices_list);
//
//	if (ret != BT_ERROR_NONE)
//	    dlog_print(DLOG_ERROR, DRG_LOG_TAG, "[bt_adapter_set_device_discovery_state_changed_cb] failed.");


	/* Classic Bluetooth */
//	ret = bt_adapter_start_device_discovery();
//	if (ret != BT_ERROR_NONE)
//		    dlog_print(DLOG_ERROR, DRG_LOG_TAG, "[bt_adapter_start_device_discovery] failed.");

//	ret = bt_device_create_bond("00:1A:7D:DA:71:13");
//	if (ret != BT_ERROR_NONE) {
//	    dlog_print(DLOG_ERROR, DRG_LOG_TAG, "[bt_device_create_bond] failed.");
//	} else {
//	    dlog_print(DLOG_INFO, DRG_LOG_TAG, "[bt_device_create_bond] succeeded. device_bond_created_cb callback will be called.");
//	}
	ret = bt_adapter_foreach_bonded_device(adapter_bonded_device_cb, remote_server_name);
	if (ret != BT_ERROR_NONE)
	    dlog_print(DLOG_ERROR, DRG_LOG_TAG, "[bt_adapter_foreach_bonded_device] failed!");

	if (bt_server_address != NULL)
	    free(bt_server_address);

	/*
	 * End of area
	 */
	return true;
}

static void _on_terminate_cb(void *user_data)
{
	uib_views_get_instance()->destroy_window_obj();
}

static void _on_resume_cb(void *user_data)
{
	/* Take necessary actions when application becomes visible. */
}

static void _on_pause_cb(void *user_data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void _on_app_control_cb(app_control_h app_control, void *user_data)
{
	/* Handle the launch request. */
}

static void _on_low_memory_cb(app_event_info_h event_info, void *user_data)
{
	/* Take necessary actions when the system runs low on memory. */
}

static void _on_low_battery_cb(app_event_info_h event_info, void *user_data)
{
	/* Take necessary actions when the battery is low. */
}

static void _on_device_orientation_cb(app_event_info_h event_info, void *user_data)
{
	/* deprecated APIs */
}

static void _on_language_changed_cb(app_event_info_h event_info, void *user_data)
{
	/* Take necessary actions is called when language setting changes. */
	uib_views_get_instance()->uib_views_current_view_redraw();
}

static void _on_region_format_changed_cb(app_event_info_h event_info, void *user_data)
{
	/* Take necessary actions when region format setting changes. */
}

