#include <sys/file.h>

#ifndef WIN32
#include <sys/ioctl.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if !( defined(WIN32) || defined(AROS) )
#include <sys/mman.h>
#endif

#include <unistd.h>
#include <fcntl.h>

#if !( defined(WIN32) || defined(ANDROIDSDL) || defined(AROS))
#include <sys/soundcard.h>
#endif

#include <sys/time.h>

#ifndef WIN32
#include <sys/resource.h>
#endif

#include <string.h>
#include <stdlib.h>

#if !( defined(WIN32) || defined(ANDROIDSDL) || defined(AROS))
#include <linux/soundcard.h>
#endif

#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "SDL.h"
#include "gp2x.h"
#include "gp2xutil.h"

#include "menu/menu.h"
#include "menu/menu_config.h"
#include "options.h"
#include "gui.h"

extern int uae4all_keystate[256];
extern void record_key(int);

int vol = 100;
unsigned long memDev;

extern SDL_Joystick *uae4all_joy0;

static int mixerdev;
int soundVolume = 50;
int flashLED;

int gp2xMouseEmuOn=0;
int gp2xButtonRemappingOn=0;
#ifndef PANDORA
int hasGp2xButtonRemapping=1;
#endif
int GFXVIDINFO_HEIGHT=240;
char launchDir[300];
char currentDir[300];

extern int graphics_init (void);

#define GPIOHOUT 0x106E
#define GPIOHPINLVL 0x118E


unsigned long gp2x_joystick_read(int allow_usb_joy)
{
	unsigned long value = 0;
	char u,d,l,r,ul,ur,dl,dr;
	
	SDL_JoystickUpdate();
	r  = (SDL_JoystickGetAxis(uae4all_joy0, 0) > 0) ? 1 : 0;
	l  = (SDL_JoystickGetAxis(uae4all_joy0, 0) < 0) ? 1 : 0;
	u  = (SDL_JoystickGetAxis(uae4all_joy0, 1) < 0) ? 1 : 0;
	d  = (SDL_JoystickGetAxis(uae4all_joy0, 1) > 0) ? 1 : 0;
	ul = (u && l) ? 1 : 0; 
	ur = (u && r) ? 1 : 0;
	dl = (d && l) ? 1 : 0;
	dr = (d && r) ? 1 : 0;
	
	if (r)  value |= GP2X_RIGHT;
	if (l)  value |= GP2X_LEFT;
	if (u)  value |= GP2X_UP;
	if (d)  value |= GP2X_DOWN;
	if (ul) value |= GP2X_UP_LEFT;
	if (ur) value |= GP2X_UP_RIGHT;
	if (dl) value |= GP2X_DOWN_LEFT;
	if (dr) value |= GP2X_DOWN_RIGHT;
	
	if (SDL_JoystickGetButton(uae4all_joy0, GP2X_BUTTON_A))	     value |= GP2X_A;
	if (SDL_JoystickGetButton(uae4all_joy0, GP2X_BUTTON_X))	     value |= GP2X_X;
	if (SDL_JoystickGetButton(uae4all_joy0, GP2X_BUTTON_Y))	     value |= GP2X_Y;
	if (SDL_JoystickGetButton(uae4all_joy0, GP2X_BUTTON_B))	     value |= GP2X_B;
	if (SDL_JoystickGetButton(uae4all_joy0, GP2X_BUTTON_L))	     value |= GP2X_L;
	if (SDL_JoystickGetButton(uae4all_joy0, GP2X_BUTTON_R))	     value |= GP2X_R;
	if (SDL_JoystickGetButton(uae4all_joy0, GP2X_BUTTON_START))  value |= GP2X_START;
	if (SDL_JoystickGetButton(uae4all_joy0, GP2X_BUTTON_SELECT)) value |= GP2X_SELECT;

	return value;
}

extern unsigned int sound_rate;

typedef struct _cmdline_opt
{
	char *optname;
	char *description;
	int len;
	void *opt;
} cmdline_opt;

static cmdline_opt cmdl_opts[] =
{
	{ "-drives"         , "   number of drives"       ,0,                           &mainMenu_drives    },
	{ "-df0"            , "      <adf filename>"      ,sizeof(uae4all_image_file0), uae4all_image_file0 },
	{ "-df1"            , "      <adf filename>"      ,sizeof(uae4all_image_file1), uae4all_image_file1 },
	{ "-df2"            , "      <adf filename>"      ,sizeof(uae4all_image_file2), uae4all_image_file2 },
	{ "-df3"            , "      <adf filename>"      ,sizeof(uae4all_image_file2), uae4all_image_file3 },
	{ "-kick"           , "     <kickstart filename>" ,sizeof(romfile),             romfile             },
	{ "-cpu"            , "      0:68000 1:68020"     ,0,                           &mainMenu_CPU_speed },
	{ "-clock"          , "    0:7Mhz 1:14Mhz 2:28Mhz",0,                           &mainMenu_CPU_model },
	{ "-chipset"        , "  0:OCS  1:ECS  2:AGA"     ,0,                           &mainMenu_chipset   },

	{ "-chipmem"        , "  0:512K 1:1M   2:2M 3:4M 4:8M",0,                           &mainMenu_chipMemory   },
	{ "-slowmem"        , "  0:Off  1:512K 2:1M 3:15M"    ,0,                           &mainMenu_slowMemory   },
	{ "-fastmem"        , "  0:Off  1:1M   2:2M 3:4M 4:8M",0,                           &mainMenu_fastMemory   },


	{ "-hdmode"         , "   0:Off 1:HDDir 2:HDFile" ,0,               &mainMenu_bootHD},

	{ "-hdfile"         , "   <HD Filename>"          ,sizeof(uae4all_hard_file),   &uae4all_hard_file},

	{ "-statusln"       , " 0:Off 1:On"               ,0,               &mainMenu_showStatus},
	{ "-sound"          , "    0:Off 1:Fast 2:Accurate",0,              &mainMenu_sound     },
	{ "-soundrate"      , "8000,11025,22050,32000 or 44100",0,          &sound_rate          },
	{ "-autosave"       , ""                         ,0,                &mainMenu_autosave         },
	{ "-frameskip"      , "number of frameskip "     ,0,                &mainMenu_frameskip        },

	{ "-width"          , "    emulation screen width",0,                &visibleAreaWidth        },
	{ "-height"         , "   emulation screen height",0,                &mainMenu_displayedLines        },

#ifdef ANDROIDSDL
	{ "-onscreen"       , ""                         ,0,                &mainMenu_onScreen         },
#endif
	{ "-ntsc"           , "     0:Pal :Ntsc"         ,0,                &mainMenu_ntsc             },
	{ "-joyconf"        , ""                         ,0,                &mainMenu_joyConf          },
	{ "-use1mbchip"     , ""                         ,0,                &mainMenu_chipMemory       },
	{ "-autofire"       , ""                         ,0,                &mainMenu_autofire         },
	{ "-script"         , ""                         ,0,                &mainMenu_enableScripts    },
	{ "-screenshot"     , ""                         ,0,                &mainMenu_enableScreenshots},
//	{ "-mousemultiplier", ""                         ,0,                &mainMenu_mouseMultiplier  },
//	{ "-skipintro"      , ""                         ,0,                &skipintro                 },
//	{ "-systemclock"    , ""                         ,0,                &mainMenu_throttle         },
//	{ "-syncthreshold"  , ""                         ,0,                &timeslice_mode            },
};


void parse_cmdline(int argc, char **argv)
{
	int arg, i, found;
	printf("Uae4all2 0.4 by Chips for Raspberry Pi\n\n");

	for (arg = 1; arg < argc; arg++)
	{
		for (i = found = 0; i < sizeof(cmdl_opts) / sizeof(cmdl_opts[0]); i++)
		{
			if (strcmp(argv[arg], cmdl_opts[i].optname) == 0)
			{
				arg++;
				if (cmdl_opts[i].len == 0)
					*(int *)(cmdl_opts[i].opt) = atoi(argv[arg]);
				else
				{
					strncpy((char *)cmdl_opts[i].opt, argv[arg], cmdl_opts[i].len);
					((char *)cmdl_opts[i].opt)[cmdl_opts[i].len-1] = 0;
				}
				found = 1;
				break;
			}
		}
		//if (!found) printf("skipping unknown option: \"%s\"\n", argv[arg]);
		if (!found)
		{
			int j;
			printf("unknown option: \"%s\"\n", argv[arg]);
			printf("Available options:\n");
			for (j = 0; j < sizeof(cmdl_opts) / sizeof(cmdl_opts[0]); j++)
			{
				printf("     %s %s\n", cmdl_opts[j], cmdl_opts[j].description);
			}
			exit (0);
		}
	}
}

void gp2x_init(int argc, char **argv)
{
	parse_cmdline(argc, argv);
	unsigned long memdev;
#ifndef AROS
	mixerdev = open("/dev/mixer", O_RDWR);
#endif
	
	SDL_ShowCursor(SDL_DISABLE);
	getcwd(launchDir, 250);
	getcwd(currentDir, 250);
	strcat(currentDir,"/roms/");
}

void gp2x_close( void )
{
}

int is_overridden_button(int button)
{
	// TODO: load from file
	return button == GP2X_BUTTON_L || button == GP2X_BUTTON_R || 
		   button == GP2X_BUTTON_A || button == GP2X_BUTTON_B ||
		   button == GP2X_BUTTON_X || button == GP2X_BUTTON_Y;
}

int get_key_for_button(int button)
{
	return 0;
}

// apply the remapped button keystroke
void handle_remapped_button_down(int button)
{
	int key = get_key_for_button(button);
	if (!uae4all_keystate[key])
	{
		uae4all_keystate[key] = 1;
		record_key(key << 1);
	}
}

void handle_remapped_button_up(int button)
{
	int key = get_key_for_button(button);

	if (uae4all_keystate[key])
	{
		uae4all_keystate[key] = 0;
		record_key((key << 1) | 1);
	}
}

void switch_to_hw_sdl(int first_time)
{
	GFXVIDINFO_HEIGHT=240;
	SDL_ShowCursor(SDL_DISABLE);
	usleep(100*1000);

	// reinit video
	graphics_init();
}

void switch_to_sw_sdl(void)
{
}

