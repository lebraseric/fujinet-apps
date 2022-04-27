/**
 * restore - copy an image to a target disk
 *
 * @author Thomas Cherryhomes
 * @email thom dot cherryhomes at gmail dot com
 * @license gpl v. 3
 */

#ifdef BUILD_ADAM

#include <eos.h>
#include <smartkeys.h>
#include "input.h"

/**
 * ADAM keyboard mapping
 */
#define KEY_BACKSPACE    0x08
#define KEY_TAB          0x09
#define KEY_RETURN       0x0D
#define KEY_ESCAPE       0x1B
#define KEY_SPACE        0x20
#define KEY_1            0x31
#define KEY_2            0x32
#define KEY_3            0x33
#define KEY_4            0x34
#define KEY_5            0x35
#define KEY_6            0x36
#define KEY_7            0x37
#define KEY_8            0x38
#define KEY_HOME         0x80
#define KEY_SMART_I      0x81
#define KEY_SMART_II     0x82
#define KEY_SMART_III    0x83
#define KEY_SMART_IV     0x84
#define KEY_SMART_V      0x85
#define KEY_SMART_VI     0x86
#define KEY_WILD_CARD    0x90
#define KEY_UNDO         0x91
#define KEY_MOVE         0x9A
#define KEY_GET          0x93
#define KEY_INSERT       0x94
#define KEY_PRINT        0x95
#define KEY_CLEAR        0x96
#define KEY_DELETE       0x97
#define KEY_COPY         0x92
#define KEY_STORE        0x9B
#define KEY_S_INSERT     0x9C
#define KEY_S_PRINT      0x9D
#define KEY_S_CLEAR      0x9E
#define KEY_S_DELETE     0x9F
#define KEY_UP_ARROW     0xA0
#define KEY_DOWN_ARROW   0xA2
#define KEY_C_UP_ARROW   0xA4
#define KEY_C_DOWN_ARROW 0xA6

static GameControllerData cont;
static unsigned char key=0;
static unsigned char joystick=0;
static unsigned char joystick_copy=0;
static unsigned char button=0;
static unsigned char button_copy=0;
static unsigned char keypad=0;
static unsigned char keypad_copy=0;
static unsigned char repeat=0;
static unsigned short entry_timer;
/**
 * Get input from keyboard/joystick
 * @return keycode (or synthesized keycode if joystick)
 */
unsigned char input()
{
  key = eos_end_read_keyboard();
  if (entry_timer>0)
    entry_timer--;

  if (key > 1)
    {
      eos_start_read_keyboard();
      if (key != 0x0D && key != 0x1B)
	smartkeys_sound_play(SOUND_KEY_PRESS);
      return key;
    }
  else
    {
      eos_read_game_controller(0x03,&cont);
      joystick = cont.joystick1 | cont.joystick2;
      button = cont.joystick1_button_left | cont.joystick1_button_right | cont.joystick2_button_left | cont.joystick2_button_right;
      keypad = cont.joystick1_keypad;

      if ((button > 0) && (button != button_copy))
	{
	  key=0x0d; // same as RETURN
	}
      else if ((keypad != 0x0F) && (keypad != keypad_copy))
	{
	  switch (keypad)
	    {
	    case 1: // Slot 1
	      key=KEY_1;
	      break;
	    case 2: // Slot 2
	      key=KEY_2;
	      break;
	    case 3: // Slot 3
	      key=KEY_3;
	      break;
	    case 4: // Slot 4
	      key=KEY_4;
	      break;
	    case 5: // Slot 5
	      key=KEY_5;
	      break;
	    case 6: // Slot 6
	      key=KEY_6;
	      break;
	    case 7: // Slot 7
	      key=KEY_7;
	      break;
	    case 8: // Slot 8
	      key=KEY_8;
	      break;
	    case 0x0a: // *
	      key=KEY_SMART_VI;
	      break;
	    }
	}
      else if ((joystick > 0) && (joystick == joystick_copy))
	repeat++;
      else
	repeat=0;

      if (repeat > 16)
	{
	  repeat=0;
	  switch(joystick)
	    {
	    case 1: // UP
	      key=KEY_UP_ARROW;
	      break;
	    case 4: // DOWN
	      key=KEY_DOWN_ARROW;
	      break;
	    }
	}
      
      joystick_copy = joystick;
      button_copy = button;
      keypad_copy = keypad;
    }
  
  return key;
}

void input_init(void)
{
  eos_start_read_keyboard();
}

bool input_select_host(char *h)
{
  unsigned char k = input();
  
  switch(k)
    {
    case KEY_1:
    case KEY_2:
    case KEY_3:
    case KEY_4:
    case KEY_5:
    case KEY_6:
    case KEY_7:
    case KEY_8:
      *h = k - KEY_1;
      break;
    case KEY_UP_ARROW:
      if (*h > 0)
	*h--;
      break;
    case KEY_DOWN_ARROW:
      if (*h < 8)
	*h++;
      break;
    case KEY_RETURN:
      return true;
    }
  
  return false;
}

#endif /* BUILD_ADAM */
