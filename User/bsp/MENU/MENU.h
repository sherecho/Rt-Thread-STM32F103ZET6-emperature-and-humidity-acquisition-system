#pragma once
#ifndef __MENU_H
#define __MENU_H
#include"bsp_lcd.h"
#include "MENU_KEY.h"

#define MAIN_STA 0
#define BRANCH_STA 1

 struct menu_sta
{
	u8 menu_loc_now;

	u8 verify_sta;

	void (*current_operation)();
};

struct menu_sta  deal_key(void);
void display_Mainlist(void);
void key_choice_handle(void);

void tem_set_display(void);
void hum_set_display(void);
void samp_set_display(void);
void show_tem_lcdwave(int index);
void lcd_showwave(void);
void humtem_infor_display(void);
#endif