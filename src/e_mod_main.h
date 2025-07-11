#ifndef E_MOD_MAIN_H
#define E_MOD_MAIN_H

#include "config.h"
#ifdef HAVE_GETTEXT
# include <libintl.h>
# define D_(string) dgettext(LOCALEDOMAIN, string)
#else
# define bindtextdomain(domain,dir)
# define bind_textdomain_codeset(domain,codeset)
# define D_(string) (string)
#endif
#define N_(string) (string)

/* Macros used for config file versioning */
/* You can increment the EPOCH value if the old configuration is not
 * compatible anymore, it creates an entire new one.
 * You need to increment GENERATION when you add new values to the
 * configuration file but is not needed to delete the existing conf  */
#define MOD_CONFIG_FILE_EPOCH 0x0001
#define MOD_CONFIG_FILE_GENERATION 0x008d
#define MOD_CONFIG_FILE_VERSION \
   ((MOD_CONFIG_FILE_EPOCH << 16) | MOD_CONFIG_FILE_GENERATION)

/* We create a structure config for our module, and also a config structure
 * for every item element (you can have multiple gadgets for the same module) */

typedef struct _Config Config;
typedef struct _Config_Item Config_Item;

/* Base config struct. Store Item Count, etc
 * 
 * *module (not written to disk) (E Interaction)
 * *cfd (not written to disk) (config dialog)
 * 
 * Store list of your items that you want to keep. (sorting)
 * Can define per-module config properties here.
 * 
 * Version used to know when user config too old */
struct _Config 
{
   /* Store a reference to the module instance provided by enlightenment in
    * e_modapi_init, in case you need to access it. (not written to disk) */
   E_Module *module;

   /* if you open a config dialog, store a reference to it in a pointer like
    * this one, so if the user opens a second time the dialog, you know it's
    * already open. Also needed for destroy the dialog when we are exiting */
   E_Config_Dialog *cfd;

   /* List of configuration items, when we save to disk, if we have a list of
    * items (Config_Item) they will be saved too */
   Eina_List *conf_items;

   /* config file version */
   int version;

   /* actual config properties; Define your own. (globally per-module) */
   unsigned char switch1;
   //double font_size;
};

/* This struct used to hold config for individual items from above list */
struct _Config_Item 
{
   /* unique id for every running gadget, this is managed by gadcon */
   const char *id;
   const char *header_text, *command, *notif_text; 
   const char *area_text_1;
   const char *area_text_2; 
   const char *area_text_3; 
   const char *area_text_4; 
   const char *area_text_5; 
   /* actual config properties independently for every running gadget. */
   int header_switch, multiply_switch, notif_switch;
   int font_size;
   double interval;
   struct
   {
      int r, g, b, a;
   } val;
};

/* Setup the E Module Version, Needed to check if module can run. */
/* The version is stored at compilation time in the module, and is checked
 * by E in order to know if the module is compatible with the actual version */
EAPI extern E_Module_Api e_modapi;

/* E API Module Interface Declarations
 *
 * e_modapi_init:     it is called when e17 initialize the module, note that
 *                    a module can be loaded but not initialized (running)
 *                    Note that this is not the same as _gc_init, that is called
 *                    when the module appears on his container
 * e_modapi_shutdown: it is called when e17 is closing, so calling the modules
 *                    to finish
 * e_modapi_save:     this is called when e17 or by another reason is requeested
 *                    to save the configuration file                      */
EAPI void *e_modapi_init(E_Module *m);
EAPI int e_modapi_shutdown(E_Module *m);
EAPI int e_modapi_save(E_Module *m);

/* Function for calling the module's Configuration Dialog */
void _config_sticky_notes_module(Config_Item * ci);
void _sticky_notes_config_updated(Config_Item *ci);
extern Config *sticky_notes_conf;

#endif
