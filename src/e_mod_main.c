#include <e.h>
#include "e_mod_main.h"
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Elementary.h>

/* Local Function Prototypes */
static E_Gadcon_Client *_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style);
static void _gc_shutdown(E_Gadcon_Client *gcc);
static void _gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient);
static const char *_gc_label(const E_Gadcon_Client_Class *client_class);
static const char *_gc_id_new(const E_Gadcon_Client_Class *client_class);
static Evas_Object *_gc_icon(const E_Gadcon_Client_Class *client_class, Evas *evas);

static Config_Item *_sticky_notes_conf_item_get(const char *id);
static void      _sticky_notes_conf_new(void);
static void      _sticky_notes_conf_free(void);
static Eina_Bool _sticky_notes_conf_timer(void *data);

static void      _sticky_notes_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event);
static void      _sticky_notes_cb_menu_post(void *data, E_Menu *menu);
static void      _sticky_notes_cb_menu_configure(void *data, E_Menu *mn, E_Menu_Item *mi);
static Eina_Bool _sticky_notes_cb_check(void *data);
static void      _sticky_header_icon_activated_cb(void *data, Evas_Object *o, const char *emission, const char *source);
static void      _sticky_header_text_activated_cb(void *data, Evas_Object *o, const char *emission, const char *source);
static void      _font_size_show(void *data, Eina_Bool save, const char *chr);
static void      _sticky_settings_activated_cb(void *data, Evas_Object *o, const char *emission, const char *source);
static Eina_Bool _change_cb(void *data, int type __UNUSED__, void *event __UNUSED__);

const char *     text_sized(void *data);
const char *     show_command_output(void *data, Eina_Bool header_clicked);


/* Local Structures */

typedef struct _Instance Instance;
struct _Instance 
{
   /* An instance of our item (module) with its elements */

   /* pointer to this gadget's container */
   E_Gadcon_Client *gcc;

   /* evas_object used to display */
   Evas_Object *o_sticky_notes;
   
   /* Timer for each gadget*/
   Ecore_Timer *timer;

   /* popup anyone ? */
   E_Menu *menu;

   /* Text buffer for each gadget*/   
   Eina_Strbuf *eina_buf;
   Eina_Strbuf *eina_temp;
   Eina_Strbuf *eina_compare;

   /* Config_Item structure. Every gadget should have one :) */
   Config_Item *ci;
};

/* Local Variables */

static Eina_List *instances = NULL;
static E_Config_DD *conf_edd = NULL;
static E_Config_DD *conf_item_edd = NULL;


Config *sticky_notes_conf = NULL;

static const E_Gadcon_Client_Class _gc_class = 
{
   GADCON_CLIENT_CLASS_VERSION, "sticky_notes", 
     {_gc_init, _gc_shutdown, _gc_orient, _gc_label, _gc_icon, 
          _gc_id_new, NULL, NULL},
   E_GADCON_CLIENT_STYLE_PLAIN
};

/* We set the version and the name, check e_mod_main.h for more details */
EAPI E_Module_Api e_modapi = {E_MODULE_API_VERSION, "Sticky_notes"};

/*
 * Module Functions
 */

/* Function called when the module is initialized */
EAPI void *
e_modapi_init(E_Module *m)
{
  char buf[4096];

   /* Location of message catalogs for localization */
   snprintf(buf, sizeof(buf), "%s/locale", e_module_dir_get(m));
   bindtextdomain(PACKAGE, buf);
   bind_textdomain_codeset(PACKAGE, "UTF-8");

   /* Location of theme to load for this module */
   snprintf(buf, sizeof(buf), "%s/e-module-sticky_notes.edj", m->dir);


   /* Display this Modules config info in the main Config Panel */

   /* starts with a category, create it if not already exists */
   e_configure_registry_category_add("advanced", 80, D_("Advanced"), 
                                     NULL, "preferences-advanced");
   /* add right-side item */
   //~ e_configure_registry_item_add("advanced/sticky_notes", 110, D_("Sticky_notes"), 
                                 //~ NULL, buf, _config_sticky_notes_module);

   /* Define EET Data Storage for the config file */
   conf_item_edd = E_CONFIG_DD_NEW("Config_Item", Config_Item);
   #undef T
   #undef D
   #define T Config_Item
   #define D conf_item_edd
   E_CONFIG_VAL(D, T, id, STR);
   E_CONFIG_VAL(D, T, header_switch, INT);
   E_CONFIG_VAL(D, T, multiply_switch, INT);
   E_CONFIG_VAL(D, T, header_text, STR);
   E_CONFIG_VAL(D, T, area_text, STR);
   E_CONFIG_VAL(D, T, area_text_2, STR);
   E_CONFIG_VAL(D, T, area_text_3, STR);
   E_CONFIG_VAL(D, T, area_text_4, STR);
   E_CONFIG_VAL(D, T, area_text_5, STR);
   E_CONFIG_VAL(D, T, font_size, INT);
   E_CONFIG_VAL(D, T, command, STR);
   E_CONFIG_VAL(D, T, notif_text, STR);
   E_CONFIG_VAL(D, T, notif_switch, INT);
   E_CONFIG_VAL(D, T, interval, DOUBLE);
   E_CONFIG_VAL(D, T, val.r, INT);
   E_CONFIG_VAL(D, T, val.g, INT);
   E_CONFIG_VAL(D, T, val.b, INT);
   E_CONFIG_VAL(D, T, val.a, INT);


   conf_edd = E_CONFIG_DD_NEW("Config", Config);
   #undef T
   #undef D
   #define T Config
   #define D conf_edd
   E_CONFIG_VAL(D, T, version, INT);
   E_CONFIG_VAL(D, T, switch1, UCHAR); /* our var from header */
   E_CONFIG_LIST(D, T, conf_items, conf_item_edd); /* the list */

   /* Tell E to find any existing module data. First run ? */
   sticky_notes_conf = e_config_domain_load("module.sticky_notes", conf_edd);
   if (sticky_notes_conf)
     {
        /* Check config version */
        if ((sticky_notes_conf->version >> 16) < MOD_CONFIG_FILE_EPOCH)
          {
             /* config too old */
        _sticky_notes_conf_free();
         ecore_timer_add(1.0, _sticky_notes_conf_timer,
                 D_("StickyNotes Module Configuration data needed "
                 "upgrading. Your old configuration<br> has been"
                 " wiped and a new set of defaults initialized. "
                 "This<br>will happen regularly during "
                 "development, so don't report a<br>bug. "
                 "This simply means the module needs "
                 "new configuration<br>data by default for "
                 "usable functionality that your old<br>"
                 "configuration simply lacks. This new set of "
                 "defaults will fix<br>that by adding it in. "
                 "You can re-configure things now to your<br>"
                 "liking. Sorry for the inconvenience.<br>"));
          }

        /* Ardvarks */
        else if (sticky_notes_conf->version > MOD_CONFIG_FILE_VERSION)
          {
             /* config too new...wtf ? */
             _sticky_notes_conf_free();
         ecore_timer_add(1.0, _sticky_notes_conf_timer, 
                 D_("Your StickyNotes Module configuration is NEWER "
                 "than the module version. This is "
                 "very<br>strange. This should not happen unless"
                 " you downgraded<br>the module or "
                 "copied the configuration from a place where"
                 "<br>a newer version of the module "
                 "was running. This is bad and<br>as a "
                 "precaution your configuration has been now "
                 "restored to<br>defaults. Sorry for the "
                 "inconvenience.<br>"));
          }
     }

   /* if we don't have a config yet, or it got erased above, 
    * then create a default one */
   if (!sticky_notes_conf) _sticky_notes_conf_new();

   /* create a link from the modules config to the module
    * this is not written */
   sticky_notes_conf->module = m;

   /* Tell any gadget containers (shelves, etc) that we provide a module
    * for the user to enjoy */
   e_gadcon_provider_register(&_gc_class);

   /* Give E the module */
   
   return m;
}

/*
 * Function to unload the module
 */
EAPI int 
e_modapi_shutdown(E_Module *m __UNUSED__)
{
    /* Kill the config dialog */
   if (sticky_notes_conf->cfd) e_object_del(E_OBJECT(sticky_notes_conf->cfd));
   //~ sticky_notes_conf->cfd = NULL;
    
   /* Unregister the config dialog from the main panel */
   e_configure_registry_item_del("advanced/sticky_notes");

   /* Remove the config panel category if we can. E will tell us.
    category stays if other items using it */
   e_configure_registry_category_del("advanced");

   /* Tell E the module is now unloaded. Gets removed from shelves, etc. */
   sticky_notes_conf->module = NULL;
   e_gadcon_provider_unregister(&_gc_class);

   /* This is called when module is unloaded */
   _sticky_notes_conf_free();
   
   /* Clean EET */
   E_CONFIG_DD_FREE(conf_item_edd);
   E_CONFIG_DD_FREE(conf_edd);
   return 1;
}

/*
 * Function to Save the modules config
 */ 
EAPI int 
e_modapi_save(E_Module *m __UNUSED__)
{
   e_config_domain_save("module.sticky_notes", conf_edd, sticky_notes_conf);
   return 1;
}

/* Local Functions */

/* Called when Gadget Controller (gadcon) says to appear in scene */
static E_Gadcon_Client *
_gc_init(E_Gadcon *gc, const char *name, const char *id, const char *style)
{
   Instance *inst = NULL;
   char buf[4096], font_size[3];
   int multi;

   /* theme file */
   snprintf(buf, sizeof(buf), "%s/e-module-sticky_notes.edj", 
            sticky_notes_conf->module->dir);

   /* New visual instance, any config ? */
   inst = E_NEW(Instance, 1);
   inst->ci = _sticky_notes_conf_item_get(id);

   /* create on-screen object */
   inst->o_sticky_notes = edje_object_add(gc->evas);
   /* we have a theme ? */
   if (!e_theme_edje_object_set(inst->o_sticky_notes, "base/theme/modules/sticky_notes", 
                                "modules/sticky_notes/main"))
     edje_object_file_set(inst->o_sticky_notes, buf, "modules/sticky_notes/main");
   inst->gcc = e_gadcon_client_new(gc, name, id, style, inst->o_sticky_notes);
   inst->gcc->data = inst;

   /* hook a mouse down. we want/have a popup menu, right ? */
   evas_object_event_callback_add(inst->o_sticky_notes, EVAS_CALLBACK_MOUSE_DOWN, 
                                  _sticky_notes_cb_mouse_down, inst);
   edje_object_signal_callback_add(inst->o_sticky_notes, "header,icon,activated", "stickynotes",
                                   _sticky_header_icon_activated_cb, inst);
   edje_object_signal_callback_add(inst->o_sticky_notes, "header,text,activated", "stickynotes",
                                   _sticky_header_text_activated_cb, inst);
   edje_object_signal_callback_add(inst->o_sticky_notes, "settings,activated", "stickynotes",
                                   _sticky_settings_activated_cb, inst);

   ecore_event_handler_add(E_EVENT_SYS_RESUME, _change_cb, inst);
   ecore_event_handler_add(ECORE_EVENT_SYSTEM_TIMEDATE_CHANGED, _change_cb, inst);
   
   multi = inst->ci->multiply_switch ? 60 : 1;
   
   if ((inst->ci->interval>0) && (inst->ci->command[0] != '\0'))
     inst->timer = ecore_timer_add(inst->ci->interval * multi, _sticky_notes_cb_check , inst);
   
   inst->eina_buf = eina_strbuf_new();
   inst->eina_temp = eina_strbuf_new();
   inst->eina_compare = eina_strbuf_new();

   /* add to list of running instances so we can cleanup later */
   instances = eina_list_append(instances, inst);
   _sticky_notes_cb_check(inst);
   
   snprintf(font_size, sizeof(font_size), "%d", inst->ci->font_size);
   _font_size_show(inst, EINA_FALSE, font_size);
   
   /* return the Gadget_Container Client */
   return inst->gcc;
}


/* Called when Gadget_Container says stop */
static void 
_gc_shutdown(E_Gadcon_Client *gcc)
{
   Instance *inst = NULL;

   if (!(inst = gcc->data)) return;
   
   instances = eina_list_remove(instances, inst);
   if (inst->timer) ecore_timer_del(inst->timer);
   
   eina_strbuf_free(inst->eina_buf);
   eina_strbuf_free(inst->eina_temp);
   eina_strbuf_free(inst->eina_compare);


   /* kill popup menu */
   if (inst->menu)
     {
        e_menu_post_deactivate_callback_set(inst->menu, NULL, NULL);
        e_object_del(E_OBJECT(inst->menu));
        inst->menu = NULL;
     }
   /* delete the visual */
   if (inst->o_sticky_notes)
     {
        /* remove mouse down callback hook */
        evas_object_event_callback_del(inst->o_sticky_notes, EVAS_CALLBACK_MOUSE_DOWN, 
                                       _sticky_notes_cb_mouse_down);
        edje_object_signal_callback_del(inst->o_sticky_notes, "header,icon,activated", "stickynotes",
                                   _sticky_header_icon_activated_cb);
        edje_object_signal_callback_del(inst->o_sticky_notes, "header,text,activated", "stickynotes",
                                   _sticky_header_text_activated_cb);
        edje_object_signal_callback_del(inst->o_sticky_notes, "settings,activated", "stickynotes",
                                   _sticky_settings_activated_cb);
        evas_object_del(inst->o_sticky_notes);
     }
   E_FREE(inst);
}

/* For when container says we are changing position */
static void 
_gc_orient(E_Gadcon_Client *gcc, E_Gadcon_Orient orient __UNUSED__)
{
   e_gadcon_client_aspect_set(gcc, 16, 16);
   e_gadcon_client_min_size_set(gcc, 200, 40);
}

/* Gadget/Module label, name for our module */
static const char *
_gc_label(const E_Gadcon_Client_Class *client_class __UNUSED__)
{
   return D_("StickyNotes");
}


/* so E can keep a unique instance per-container */
static const char *
_gc_id_new(const E_Gadcon_Client_Class *client_class __UNUSED__)
{
   Config_Item *ci = NULL;

   ci = _sticky_notes_conf_item_get(NULL);
   return ci->id;
}

static Evas_Object *
_gc_icon(const E_Gadcon_Client_Class *client_class __UNUSED__, Evas *evas)
{
   Evas_Object *o = NULL;
   char buf[4096];

   /* theme */
   snprintf(buf, sizeof(buf), "%s/e-module-sticky_notes.edj", sticky_notes_conf->module->dir);

   /* create icon object */
   o = edje_object_add(evas);

   /* load icon from theme */
   edje_object_file_set(o, buf, "icon");

   return o;
}

/* new module needs a new config :), or config too old and we need one anyway */
static void 
_sticky_notes_conf_new(void)
{
   Config_Item *ci = NULL;
   
   sticky_notes_conf = E_NEW(Config, 1);
   ci = E_NEW(Config_Item, 1);
   
   sticky_notes_conf->version = (MOD_CONFIG_FILE_EPOCH << 16);

#define IFMODCFG(v) if ((sticky_notes_conf->version & 0xffff) < v) {
#define IFMODCFGEND }

   /* setup defaults */
   IFMODCFG(0x008d);
   sticky_notes_conf->switch1 = 1;
   
   //~ ci->id = eina_stringshare_add(id);
   ci->header_switch = 0;
   ci->multiply_switch = 0;
   ci->notif_switch = 0;
   ci->font_size = 12;
   ci->interval = 0.0;
   ci->val.r=255;
   ci->val.g=255;
   ci->val.b=255;
   ci->val.a=255;
   ci->header_text = eina_stringshare_add(D_("Sticky note"));
   ci->command = eina_stringshare_add("");
   ci->area_text = eina_stringshare_add("");
   ci->area_text_2 = eina_stringshare_add("");
   ci->area_text_3 = eina_stringshare_add("");
   ci->area_text_4 = eina_stringshare_add("");
   ci->area_text_5 = eina_stringshare_add("");
   ci->notif_text = eina_stringshare_add("");
   
   _sticky_notes_conf_item_get(NULL);
   IFMODCFGEND;

   /* update the version */
   sticky_notes_conf->version = MOD_CONFIG_FILE_VERSION;

   /* setup limits on the config properties here (if needed) */

   /* save the config to disk */
   e_config_save_queue();
}

/* This is called when we need to cleanup the actual configuration,
 * for example when our configuration is too old */
static void 
_sticky_notes_conf_free(void)
{
   /* cleanup any stringshares here */
   while (sticky_notes_conf->conf_items)
     {
        Config_Item *ci = NULL;
        
        /* Grab an item from the list */
        ci = sticky_notes_conf->conf_items->data;
        /* remove it */
        sticky_notes_conf->conf_items = 
          eina_list_remove_list(sticky_notes_conf->conf_items, 
                                sticky_notes_conf->conf_items);

        /* cleanup stringshares */
        if (ci->id) eina_stringshare_del(ci->id);
        if (ci->header_text) eina_stringshare_del(ci->header_text);
        if (ci->area_text) eina_stringshare_del(ci->area_text);
        if (ci->area_text_2) eina_stringshare_del(ci->area_text_2);
        if (ci->area_text_3) eina_stringshare_del(ci->area_text_3);
        if (ci->area_text_4) eina_stringshare_del(ci->area_text_4);
        if (ci->area_text_5) eina_stringshare_del(ci->area_text_5);
        if (ci->command) eina_stringshare_del(ci->command);
        if (ci->notif_text) eina_stringshare_del(ci->notif_text);
        E_FREE(ci);
     }
     
   E_FREE(sticky_notes_conf);
}

/* timer for the config oops dialog (old configuration needs update) */
static Eina_Bool 
_sticky_notes_conf_timer(void *data)
{
   e_util_dialog_internal( D_("StickyNotes Configuration Updated"), data);
   return EINA_FALSE;
}

/* function to search for any Config_Item struct for this Item
 * create if needed */
static Config_Item *
_sticky_notes_conf_item_get(const char *id)
{
   Eina_List *l;
   Config_Item *ci;
   char buf[128];

   if (!id)
     {
    int  num = 0;

    /* Create id */
    if (sticky_notes_conf->conf_items)
      {
         const char *p;

         ci = eina_list_last (sticky_notes_conf->conf_items)->data;
         p = strrchr (ci->id, '.');
         if (p) num = atoi (p + 1) + 1;
      }
    snprintf (buf, sizeof (buf), "%s.%d", _gc_class.name, num);
    id = buf;
     }
   else
     {
    for (l = sticky_notes_conf->conf_items; l; l = l->next)
      {
         if (!(ci = l->data)) continue;
         if (!strcmp (ci->id, id)) return ci;
      }
     }

   //GADCON_CLIENT_CONFIG_GET(Config_Item, sticky_notes_conf->conf_items, _gc_class, id);

   ci = E_NEW(Config_Item, 1);
   ci->id = eina_stringshare_add(id);
   ci->header_switch = 0;
   ci->multiply_switch = 0;
   ci->notif_switch = 0;
   ci->font_size = 12;
   ci->interval = 0.0;
   ci->val.r=255;
   ci->val.g=255;
   ci->val.b=255;
   ci->val.a=255;
   ci->header_text = eina_stringshare_add(D_("Sticky note"));
   ci->command = eina_stringshare_add("");
   ci->area_text = eina_stringshare_add("");
   ci->area_text_2 = eina_stringshare_add("");
   ci->area_text_3 = eina_stringshare_add("");
   ci->area_text_4 = eina_stringshare_add("");
   ci->area_text_5 = eina_stringshare_add("");
   ci->notif_text = eina_stringshare_add("");
   sticky_notes_conf->conf_items = eina_list_append(sticky_notes_conf->conf_items, ci);
   return ci;
}

/* Pants On */
static void 
_sticky_notes_cb_mouse_down(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event)
{
   Instance *inst = NULL;
   Evas_Event_Mouse_Down *ev;
   E_Zone *zone = NULL;
   E_Menu_Item *mi = NULL;
   int x, y;

   if (!(inst = data)) return;
   ev = event;
   if ((ev->button == 3) && (!inst->menu))
     {
        E_Menu *m;

        /* grab current zone */
        zone = e_util_zone_current_get(e_manager_current_get());

        /* create popup menu */
        m = e_menu_new();
        mi = e_menu_item_new(m);
        e_menu_item_label_set(mi, D_("Settings"));
        e_util_menu_item_theme_icon_set(mi, "preferences-system");
        e_menu_item_callback_set(mi, _sticky_notes_cb_menu_configure, inst);

        /* Each Gadget Client has a utility menu from the Container */
        m = e_gadcon_client_util_menu_items_append(inst->gcc, m, 0);
        e_menu_post_deactivate_callback_set(m, _sticky_notes_cb_menu_post, inst);
        inst->menu = m;
        e_gadcon_canvas_zone_geometry_get(inst->gcc->gadcon, &x, &y, 
                                          NULL, NULL);

        /* show the menu relative to gadgets position */
        e_menu_activate_mouse(m, zone, (x + ev->output.x), 
                              (y + ev->output.y), 1, 1, 
                              E_MENU_POP_DIRECTION_AUTO, ev->timestamp);
        evas_event_feed_mouse_up(inst->gcc->gadcon->evas, ev->button, 
                                 EVAS_BUTTON_NONE, ev->timestamp, NULL);
     }
}

/* popup menu closing, cleanup */
static void 
_sticky_notes_cb_menu_post(void *data, E_Menu *menu __UNUSED__)
{
   Instance *inst = NULL;

   if (!(inst = data)) return;
   
   if (!inst->menu) return;
   e_object_del(E_OBJECT(inst->menu));
   inst->menu = NULL;
}

/* call configure from popup */
static void 
_sticky_notes_cb_menu_configure(void *data, E_Menu *mn __UNUSED__, E_Menu_Item *mi __UNUSED__)
{ 
   Instance *inst = NULL;
   
   if (!(inst = data)) return;
   _config_sticky_notes_module(inst->ci);
}

void
_sticky_notes_config_updated(Config_Item *ci)
{
   Eina_List *l;
   int multi;

   if (!sticky_notes_conf) return;
   for (l = instances; l; l = l->next)
     {
        Instance *inst;

        inst = l->data;
        if (inst->ci != ci) continue;
        
        ecore_timer_del(inst->timer);
        multi = inst->ci->multiply_switch ? 60 : 1;
        
        if (inst->ci->interval>0)
           inst->timer = ecore_timer_add(inst->ci->interval * multi, _sticky_notes_cb_check, inst);
            
        if (inst->ci->command[0] == '\0')
           ecore_timer_del(inst->timer);

        _sticky_notes_cb_check(inst);
     }
}

static Eina_Bool
_sticky_notes_cb_check(void *data)
{
   Instance *inst = data;
  
   //~ Eina_List *l;
   
   //~ Uncommented lines was a solution to have timer for each gadget separatelly
           
   //~ for (l = instances; l; l = l->next)
     //~ {
        //~ inst = l->data;
        
        if ((inst->ci->header_text) && (!inst->ci->header_switch))
          edje_object_part_text_set(inst->o_sticky_notes, "header_text", inst->ci->header_text);
        else
          edje_object_part_text_set(inst->o_sticky_notes, "header_text", inst->ci->area_text);

        if (inst->ci->command[0] != '\0')
          edje_object_part_text_set(inst->o_sticky_notes, "area_text", show_command_output(inst, EINA_TRUE));

        if ((inst->ci->area_text) && (inst->ci->command[0] == '\0'))
          edje_object_part_text_set(inst->o_sticky_notes, "area_text", text_sized(inst));

        //~ snprintf(buf, sizeof(buf),"R:%d,G:%d,B:%d,A:%d",inst->ci->val.r, inst->ci->val.g, inst->ci->val.b, inst->ci->val.a);
        //~ e_util_dialog_internal("RGBA", buf);
        edje_object_color_class_set
        (inst->o_sticky_notes, "color",
        inst->ci->val.r, inst->ci->val.g, inst->ci->val.b, inst->ci->val.a,
        0, 0, 0, 255, 0, 0, 0, 255);

     //~ }

   return EINA_TRUE;
}

void
_sticky_settings_activated_cb(void *data, Evas_Object *o __UNUSED__, 
            const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Instance *inst = data;
   _sticky_notes_cb_menu_configure(inst, NULL, NULL);
}

void
_sticky_header_icon_activated_cb(void *data, Evas_Object *o __UNUSED__,
            const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Instance *inst = data;
    
   if (inst->ci->font_size<16)
     inst->ci->font_size++;
   else
     inst->ci->font_size = 8;

   if (!(inst=data)) return;

   if (inst->ci->command[0] != '\0')
     edje_object_part_text_set(inst->o_sticky_notes, "area_text", show_command_output(inst, EINA_FALSE));

   if ((inst->ci->area_text) && (inst->ci->command[0] == '\0'))
      edje_object_part_text_set(inst->o_sticky_notes, "area_text", text_sized(inst));

   _font_size_show(inst, EINA_TRUE, "");
}

void
_sticky_header_text_activated_cb(void *data, Evas_Object *o __UNUSED__,
            const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Instance *inst = data;

   if (!(inst=data)) return;

   if (inst->ci->command[0] != '\0')
     edje_object_part_text_set(inst->o_sticky_notes, "area_text", show_command_output(inst, EINA_FALSE));
}

void
_font_size_show(void *data, Eina_Bool save, const char *chr)
{
   Instance *inst = data;
   char buf[64];

   if (inst->ci->font_size<10)
     snprintf(buf, sizeof(buf), " %d", inst->ci->font_size);
   else
     snprintf(buf, sizeof(buf), "%d", inst->ci->font_size);

   edje_object_signal_emit(inst->o_sticky_notes, "size_visible", "");
   edje_object_message_signal_process(inst->o_sticky_notes);

   if (!(strcmp(chr,"")))
     edje_object_part_text_set(inst->o_sticky_notes, "font_size", buf);
   else
     edje_object_part_text_set(inst->o_sticky_notes, "font_size", chr);

   edje_object_signal_emit(inst->o_sticky_notes, "size_hidden", "");
   if (save) e_config_save_queue();
}

const char *
text_sized(void *data)
{
   Instance *inst = data;
   char buf[16];

   eina_strbuf_reset(inst->eina_buf);

   snprintf(buf, sizeof(buf), "<font_size= %d>", (int)inst->ci->font_size);
   eina_strbuf_append(inst->eina_buf, buf);
   eina_strbuf_append(inst->eina_buf, inst->ci->area_text);
   eina_strbuf_append(inst->eina_buf, "<br>");
   eina_strbuf_append(inst->eina_buf, inst->ci->area_text_2);
   eina_strbuf_append(inst->eina_buf, "<br>");
   eina_strbuf_append(inst->eina_buf, inst->ci->area_text_3);
   eina_strbuf_append(inst->eina_buf, "<br>");
   eina_strbuf_append(inst->eina_buf, inst->ci->area_text_4);
   eina_strbuf_append(inst->eina_buf, "<br>");
   eina_strbuf_append(inst->eina_buf, inst->ci->area_text_5);
   eina_strbuf_append(inst->eina_buf, "</font_size>");

   return eina_strbuf_string_get(inst->eina_buf);
}

static char *
get_time()
{
   time_t rawtime;
   struct tm * timeinfo;
   char buf[64] = "";
   time(&rawtime);
   timeinfo = localtime( &rawtime );

   snprintf(buf, sizeof(buf), "%01d", timeinfo->tm_mday);
   return strdup(buf);
}

const char *
show_command_output(void *data, Eina_Bool header_clicked)
{
   Instance *inst = data;
   FILE *output;
   char line[256], buf[16];

   eina_strbuf_reset(inst->eina_buf);
   eina_strbuf_reset(inst->eina_compare);
   eina_strbuf_append(inst->eina_compare, eina_strbuf_string_get(inst->eina_temp));

   output = popen(inst->ci->command, "r");

   snprintf(buf, sizeof(buf), "<font_size= %d>", (int)inst->ci->font_size);
   eina_strbuf_append(inst->eina_buf, buf);

   /* Reading command output to the eina buffer*/
   while (fgets(line, 256, output) != NULL)
     eina_strbuf_append(inst->eina_buf, elm_entry_utf8_to_markup(line));

   eina_strbuf_reset(inst->eina_temp);
   eina_strbuf_append(inst->eina_temp, eina_strbuf_string_get(inst->eina_buf));
   pclose(output);

   eina_strbuf_append(inst->eina_buf, "</font_size>");

   /*condition if the command is ncal. If yes, format day number to BOLD*/
   if (!strncmp(inst->ci->command, "ncal", 4))
    {
      int i, position;
      char *ret;
      char *day;
      char day_a[16], day_b[16];

      day = get_time();
      sprintf(day_a, " %s ", day);
      sprintf(day_b, "|<b>%s</b>|", day);
      eina_strbuf_replace(inst->eina_buf, day_a, day_b, 1);
    }

   /*condition if the text has been changed*/
   if (eina_strbuf_length_get(inst->eina_compare) > 0 &&
       strcmp(eina_strbuf_string_get(inst->eina_compare), eina_strbuf_string_get(inst->eina_temp)) != 0)
     {
       char cmd[200];
       if ((inst->ci->notif_switch) && (header_clicked))
         {
            snprintf(cmd, 200, "notify-send --expire-time=5000 --icon=%s 'StickyNote' '%s'",
                                         "accessories-text-editor", inst->ci->notif_text);
            ecore_init();
            ecore_exe_run(cmd, NULL);
            ecore_shutdown();
         }
     }

   return eina_strbuf_string_get(inst->eina_buf);
}

static Eina_Bool
_change_cb(void *data, int type __UNUSED__, void *event __UNUSED__)
{
    Instance *inst = data;
   _sticky_header_text_activated_cb(inst, NULL, NULL, NULL);
   return ECORE_CALLBACK_PASS_ON;
}
