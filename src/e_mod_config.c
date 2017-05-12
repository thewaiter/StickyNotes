#include <e.h>
#include <Elementary.h>

#include "e_mod_main.h"

/* The typedef for this structure is declared inside the E code in order to
 * allow everybody to use this type, you dont need to declare the typedef, 
 * just use the E_Config_Dialog_Data for your data structures declarations */
struct _E_Config_Dialog_Data 
{
   int   header_switch, multiply_switch, notif_switch;
   char *header_text, *command, *notif_text;
   char *area_text;
   char *area_text_2; 
   char *area_text_3; 
   char *area_text_4; 
   char *area_text_5; 
   double font_size, interval;
      

 struct
   {
      Evas_Object *header_label;
      Evas_Object *header_text;
      Evas_Object *header_switcher;
      
      Evas_Object *area_label;
      Evas_Object *notif_entry;
      
      Evas_Object *area_entry1;
      Evas_Object *area_entry2;
      Evas_Object *area_entry3;
      Evas_Object *area_entry4;
      Evas_Object *area_entry5;
      Evas_Object *html_label;
   } ui;
};

/* Local Function Prototypes */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _fill_data(Config_Item * ci, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void  _cb_check_changed(void *data, Evas_Object *obj __UNUSED__);
static void  _cb_check_changed_notif(void *data, Evas_Object *obj __UNUSED__);
static void  _cb_check_changed_command(void *data, Evas_Object *obj __UNUSED__);

/* External Functions */

/* Function for calling our personal dialog menu */
void
_config_sticky_notes_module(Config_Item * ci) 
{
   E_Config_Dialog *cfd = NULL;
   E_Config_Dialog_View *v = NULL;
   E_Container *con;
   char buf[4096];

   /* is this config dialog already visible ? */
   if (e_config_dialog_find("Sticky_notes", "advanced/sticky_notes")) 
     return;

   v = E_NEW(E_Config_Dialog_View, 1);
   if (!v) return;

   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.create_widgets = _basic_create;
   v->basic.apply_cfdata = _basic_apply;

   /* Icon in the theme */
   snprintf(buf, sizeof(buf), "%s/e-module-sticky_notes.edj", sticky_notes_conf->module->dir);
   con = e_container_current_get (e_manager_current_get ());
   /* create our config dialog */
   cfd = e_config_dialog_new(con, D_("StickyNotes Module"), "Sticky_notes", 
                             "advanced/sticky_notes", buf, 0, v, ci);

   e_dialog_resizable_set(cfd->dia, 1);
   sticky_notes_conf->cfd = cfd;
}

/* Local Functions */
static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata = NULL;
   Config_Item *ci;
   ci = cfd->data;
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(ci, cfdata);
   return cfdata;
}

static void 
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   sticky_notes_conf->cfd = NULL;
   E_FREE(cfdata);
}

static void 
_fill_data(Config_Item * ci, E_Config_Dialog_Data *cfdata) 
{
   /* load a temp copy of the config variables */
    cfdata->header_switch = ci->header_switch;
    if (ci->header_text) cfdata->header_text = strdup(ci->header_text);
    if (ci->area_text) cfdata->area_text = strdup(ci->area_text);
    if (ci->area_text_2) cfdata->area_text_2 = strdup(ci->area_text_2);
    if (ci->area_text_3) cfdata->area_text_3 = strdup(ci->area_text_3);
    if (ci->area_text_4) cfdata->area_text_4 = strdup(ci->area_text_4);
    if (ci->area_text_5) cfdata->area_text_5 = strdup(ci->area_text_5);
    if (ci->command) cfdata->command = strdup(ci->command);
    if (ci->notif_text) cfdata->notif_text = strdup(ci->notif_text);
    cfdata->interval = ci->interval;
    cfdata->multiply_switch = ci->multiply_switch;
    cfdata->notif_switch = ci->notif_switch;
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o = NULL, *of = NULL, *ow = NULL, *box = NULL;
   
   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, D_("General"), 0);
   e_widget_framelist_content_align_set(of, 0.0, 0.0);
   
   ow = e_widget_check_add(evas, D_(" Note text in the header"), &(cfdata->header_switch));
   cfdata->ui.header_switcher = ow;
   e_widget_framelist_object_append(of, ow);
   e_widget_on_change_hook_set(ow, _cb_check_changed, cfdata);
      
   ow = e_widget_label_add (evas, D_("Header text"));
   cfdata->ui.header_label = ow;
   e_widget_framelist_object_append (of, ow);
   
   ow = e_widget_entry_add(evas, &(cfdata->header_text), NULL, NULL, NULL);
   cfdata->ui.header_text = ow;
   e_widget_framelist_object_append (of, ow);

   ow = e_widget_label_add (evas, D_("Note text"));
   cfdata->ui.area_label = ow;
   e_widget_framelist_object_append(of, ow);
   
   ow = e_widget_entry_add(evas, &(cfdata->area_text), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ow);
   cfdata->ui.area_entry1 = ow;
   ow = e_widget_entry_add(evas, &(cfdata->area_text_2), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ow);
   cfdata->ui.area_entry2 = ow;
   ow = e_widget_entry_add(evas, &(cfdata->area_text_3), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ow);
   cfdata->ui.area_entry3 = ow;
   ow = e_widget_entry_add(evas, &(cfdata->area_text_4), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ow);
   cfdata->ui.area_entry4 = ow;
   ow = e_widget_entry_add(evas, &(cfdata->area_text_5), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ow);
   cfdata->ui.area_entry5 = ow;

   
   ow = e_widget_label_add (evas, D_("HTML tags: <b>, <i>, <br>, <ps>, <tab>"));
   cfdata->ui.html_label = ow;
   e_widget_framelist_object_append(of, ow);
  
   e_widget_list_object_append(o, of, 1, 0, 0.5);
  
   of = e_widget_framelist_add(evas, D_("Command section"), 0);
  
   ow = e_widget_label_add (evas, D_("Command to run and show output"));
   e_widget_size_min_set(ow, 120, 25);
   e_widget_framelist_object_append(of, ow);
   
   ow = e_widget_entry_add(evas, &(cfdata->command), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ow);
   e_widget_on_change_hook_set(ow, _cb_check_changed_command, cfdata);
    
   
   ow = e_widget_label_add(evas, D_("Refresh interval"));
   e_widget_framelist_object_append(of, ow);
   
   ow = e_widget_slider_add(evas, 1, 0, D_("%2.0f sec"), 0.0, 60.0, 1.0, 0,&(cfdata->interval), NULL, 40);
   e_widget_framelist_object_append(of, ow);
   
   ow = e_widget_check_add(evas, D_(" x 60 secs"), &(cfdata->multiply_switch));
   e_widget_framelist_object_append(of, ow);
   
   ow = e_widget_check_add(evas, D_(" Notification text on change"), &(cfdata->notif_switch));
   e_widget_framelist_object_append(of, ow);
   e_widget_on_change_hook_set(ow, _cb_check_changed_notif, cfdata);
   
   ow = e_widget_entry_add(evas, &(cfdata->notif_text), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ow);
   cfdata->ui.notif_entry = ow;
   
   e_widget_list_object_append(o, of, 1, 0, 0.5);
   _cb_check_changed(cfdata, NULL);
   _cb_check_changed_command(cfdata, NULL);
   _cb_check_changed_notif(cfdata, NULL);
   
    e_dialog_resizable_set(cfd->dia, EINA_FALSE);
   return o;
}

static void  
_cb_check_changed(void *data, Evas_Object *obj __UNUSED__)
{
	E_Config_Dialog_Data *cfdata;
	if (!(cfdata = data)) return;
    
    if(cfdata->header_switch) {
      e_widget_disabled_set(cfdata->ui.header_label, EINA_TRUE);
      e_widget_disabled_set(cfdata->ui.header_text, EINA_TRUE);
    } else {
      e_widget_disabled_set(cfdata->ui.header_label, EINA_FALSE);
      e_widget_disabled_set(cfdata->ui.header_text, EINA_FALSE);
    }
}

static void  
_cb_check_changed_notif(void *data, Evas_Object *obj __UNUSED__)
{
	E_Config_Dialog_Data *cfdata;
	if (!(cfdata = data)) return;
    if(cfdata->notif_switch) 
      e_widget_disabled_set(cfdata->ui.notif_entry, EINA_FALSE);
    else 
      e_widget_disabled_set(cfdata->ui.notif_entry, EINA_TRUE);
}

static void  _cb_check_changed_command(void *data, Evas_Object *obj __UNUSED__)
{
	E_Config_Dialog_Data *cfdata;
	if (!(cfdata = data)) return;
	
	if(strlen(cfdata->command)) {
	  if (cfdata->header_switch)
	  e_widget_check_checked_set(cfdata->ui.header_switcher, 0);
  	  e_widget_disabled_set(cfdata->ui.header_switcher, EINA_TRUE);

	  e_widget_disabled_set(cfdata->ui.header_text, EINA_FALSE);
	  e_widget_disabled_set(cfdata->ui.area_label, EINA_TRUE);
      e_widget_disabled_set(cfdata->ui.area_entry1, EINA_TRUE);
      e_widget_disabled_set(cfdata->ui.area_entry2, EINA_TRUE);
      e_widget_disabled_set(cfdata->ui.area_entry3, EINA_TRUE);
      e_widget_disabled_set(cfdata->ui.area_entry4, EINA_TRUE);
      e_widget_disabled_set(cfdata->ui.area_entry5, EINA_TRUE);
      e_widget_disabled_set(cfdata->ui.html_label, EINA_TRUE);
    } else {
	  e_widget_disabled_set(cfdata->ui.header_switcher, EINA_FALSE);
  	  e_widget_disabled_set(cfdata->ui.area_label, EINA_FALSE);
      e_widget_disabled_set(cfdata->ui.area_entry1, EINA_FALSE);
      e_widget_disabled_set(cfdata->ui.area_entry2, EINA_FALSE);
      e_widget_disabled_set(cfdata->ui.area_entry3, EINA_FALSE);
      e_widget_disabled_set(cfdata->ui.area_entry4, EINA_FALSE);
      e_widget_disabled_set(cfdata->ui.area_entry5, EINA_FALSE);
      e_widget_disabled_set(cfdata->ui.html_label, EINA_FALSE);
    }
}

static int 
_basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   Config_Item *ci = NULL;
   ci = cfd->data;
   ci->header_switch = cfdata->header_switch;
   if (ci->header_text) eina_stringshare_del(ci->header_text);
   ci->header_text = eina_stringshare_add(cfdata->header_text);
   if (ci->area_text) eina_stringshare_del(ci->area_text);
   ci->area_text = eina_stringshare_add(cfdata->area_text);
   if (ci->area_text_2) eina_stringshare_del(ci->area_text_2);
   ci->area_text_2 = eina_stringshare_add(cfdata->area_text_2);
   if (ci->area_text_3) eina_stringshare_del(ci->area_text_3);
   ci->area_text_3 = eina_stringshare_add(cfdata->area_text_3);
   if (ci->area_text_4) eina_stringshare_del(ci->area_text_4);
   ci->area_text_4 = eina_stringshare_add(cfdata->area_text_4);
   if (ci->area_text_5) eina_stringshare_del(ci->area_text_5);
   ci->area_text_5 = eina_stringshare_add(cfdata->area_text_5);
   if (ci->command) eina_stringshare_del(ci->command);
   ci->command = eina_stringshare_add(cfdata->command);
   if (ci->notif_text) eina_stringshare_del(ci->notif_text);
   ci->notif_text = eina_stringshare_add(cfdata->notif_text);
   
   ci->interval = cfdata->interval;
   ci->multiply_switch = cfdata->multiply_switch;
   ci->notif_switch = cfdata->notif_switch;
   
   e_config_save_queue();
   _sticky_notes_config_updated(ci);
   return 1;
}
