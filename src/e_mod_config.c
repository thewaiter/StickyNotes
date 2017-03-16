#include <e.h>
#include <Elementary.h>

#include "e_mod_main.h"

/* The typedef for this structure is declared inside the E code in order to
 * allow everybody to use this type, you dont need to declare the typedef, 
 * just use the E_Config_Dialog_Data for your data structures declarations */
struct _E_Config_Dialog_Data 
{
   int   header_switch;
   char *header_text, *area_text;

 struct
   {
      Evas_Object *header_label;
      Evas_Object *header_text;
   } ui;
};

/* Local Function Prototypes */
static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void _fill_data(Config_Item * ci, E_Config_Dialog_Data *cfdata);
static Evas_Object *_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int _basic_apply(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static void  _cb_check_changed(void *data, Evas_Object *obj __UNUSED__);

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
}

static Evas_Object *
_basic_create(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o = NULL, *of = NULL, *ow = NULL, *box = NULL;
   
   o = e_widget_list_add(evas, 0, 0);

   of = e_widget_framelist_add(evas, D_("Settings"), 0);
   e_widget_framelist_content_align_set(of, 0.0, 0.0);
   
   ow = e_widget_check_add(evas, D_(" Note text in the header"), &(cfdata->header_switch));
   e_widget_framelist_object_append(of, ow);
   e_widget_on_change_hook_set(ow, _cb_check_changed, cfdata);
      
   ow = e_widget_label_add (evas, D_("Header text"));
   cfdata->ui.header_label = ow;
   e_widget_framelist_object_append (of, ow);
   
   ow = e_widget_entry_add(evas, &(cfdata->header_text), NULL, NULL, NULL);
   cfdata->ui.header_text = ow;
   e_widget_framelist_object_append (of, ow);
   
   ow = e_widget_label_add (evas, D_("Note text"));
   e_widget_size_min_set(ow, 120, 25);
   e_widget_framelist_object_append(of, ow);
   
   //~ ow = elm_entry_add(of);
   //~ elm_entry_entry_set(ow, "A short text.");
   //~ elm_entry_scrollable_set(ow, EINA_TRUE);
   //~ elm_entry_editable_set(ow, EINA_TRUE);
   //~ elm_entry_cursor_begin_set(ow);
   //~ evas_object_show(ow);

   //~ e_widget_framelist_object_append(of, ow);

   //~ box = elm_box_add(of);
   //~ elm_win_resize_object_add(of, box);
   //~ evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //~ evas_object_show(box);
//~ 
   //~ ow = elm_entry_add(box);
   
   //~ elm_entry_entry_set(ow, "A short text.");
   //~ elm_object_text_set(ow, "A short text.");
   //elm_entry_scrollable_set(ow, EINA_TRUE);
   //~ elm_entry_editable_set(ow, EINA_TRUE);
   //elm_entry_line_wrap_set(ow, _ent_cfg->wrap_type);
   //elm_entry_cnp_mode_set(ow, ELM_CNP_MODE_PLAINTEXT);
   //~ evas_object_size_hint_align_set(ow, EVAS_HINT_FILL, EVAS_HINT_FILL);
   //~ evas_object_size_hint_weight_set(ow, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   //elm_box_pack_end(bx, main_ec_ent->entry);
   //~ evas_object_show(ow);

   ow = e_widget_entry_add(evas, &(cfdata->area_text), NULL, NULL, NULL);
   e_widget_framelist_object_append(of, ow);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   _cb_check_changed(cfdata,NULL);
    e_dialog_resizable_set(cfd->dia, EINA_TRUE);
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
   
   e_config_save_queue();
   _sticky_notes_config_updated(ci);
   return 1;
}
