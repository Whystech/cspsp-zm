#include "GameStateOptions.h"

GameStateOptions::GameStateOptions(GameApp* parent): GameState(parent) 
{
	mState = OPTIONS;
	index = 0;
	main = true;
	gLastKey = 0;
	gKeyRepeatDelay = 0;
}

GameStateOptions::~GameStateOptions() {}

void GameStateOptions::Create()
{
	movementstyle = ABSOLUTE1;
	music = OFF;
	friendlyfire = ON;
	menustyle = ANALOG;
	respawnstyle = RESPAWN_BASE;
	freezetime = FREEZE_TIME_3;
	horderegroup = HORDE_REGROUP_BASE;
	hordewavedelay = HORDE_DELAY_3;


	//initial check to use nickname
	#ifdef WIN32
	#else
	char* nameconfig = GetConfig("data/config.txt","name");
	if (nameconfig != NULL) {
		strncpy(name,nameconfig,15);
		delete nameconfig;
	}
	else {
		strcpy(name,"default");
	}
	name[15] = '\0';
	FILE* file = fopen("nataku92.txt","r");
	if (file == NULL) {
		if (stricmp(name,"nataku92") == 0 || strstr(name,"nataku92") != NULL) {
			strcpy(name,"default");
		}
	}
	else {
		fclose(file);
	}

	if (strcmp(name,"default") == 0) {
		sceUtilityGetSystemParamString(PSP_SYSTEMPARAM_ID_STRING_NICKNAME, name, 15);
		name[15] = '\0';

		movementstyle = ABSOLUTE1;
		char* movement = GetConfig("data/config.txt","movement");
		if (movement != NULL) {
			if (strcmp(movement,"relative") == 0) {
				movementstyle = RELATIVE1;
			}
			else if (strcmp(movement,"absolute") == 0) {
				movementstyle = ABSOLUTE1;
			}
			delete movement;
		}

		music = OFF;
		char* musicconfig = GetConfig("data/config.txt","music");
		if (musicconfig != NULL) {
			if (strcmp(musicconfig,"on") == 0) {
				music = ON;
			}
			else if (strcmp(musicconfig,"off") == 0) {
				music = OFF;
			}
			delete musicconfig;
		}

		friendlyfire = ON;
		char* ff = GetConfig("data/config.txt","ff");
		if (ff != NULL) {
			if (strcmp(ff,"on") == 0) {
				friendlyfire = ON;
			}
			else if (strcmp(ff,"off") == 0) {
				friendlyfire = OFF;
			}
			delete ff;
		}

		menustyle = ANALOG;
		char* menu = GetConfig("data/config.txt","menu");
		if (menu != NULL) {
			if (strcmp(menu,"analog") == 0) {
				menustyle = ANALOG;
			}
			else if (strcmp(menu,"dirpad") == 0) {
				menustyle = DIRPAD;
			}
			delete menu;
		}

		Save();
	}
	#endif


	for (int i=0; i<NUMCONFIGS; i++) {
		mGuiControllers[i] = NULL;
		strcpy(mConfigs[i],"");
		strcpy(mConfigInfo[i],"");
	}
	float x = 160+10;

	mGuiControllers[0] = new JGuiController(0, this, JGUI_STYLE_LEFTRIGHT);
	mGuiControllers[0]->Add(new MenuItem(RELATIVE1, gFont, "Relative", x, 38, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[0]->Add(new MenuItem(ABSOLUTE1, gFont, "Absolute", x+100, 38, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[0]->SetActive(false);
	strcpy(mConfigs[0],"Movement Style");
	strcpy(mConfigInfo[0],"Change your player's movement style");

	mGuiControllers[1] = new JGuiController(1, this, JGUI_STYLE_LEFTRIGHT);
	mGuiControllers[1]->Add(new MenuItem(ON, gFont, "on", x, 55, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[1]->Add(new MenuItem(OFF, gFont, "off", x+50, 55, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[1]->SetActive(false);
	strcpy(mConfigs[1],"Music");
	strcpy(mConfigInfo[1],"Enable or disable the background music in Singleplayer");

	mGuiControllers[2] = new JGuiController(2, this, JGUI_STYLE_LEFTRIGHT);
	mGuiControllers[2]->Add(new MenuItem(ON, gFont, "on", x, 72, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[2]->Add(new MenuItem(OFF, gFont, "off", x+50, 72, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[2]->SetActive(false);
	strcpy(mConfigs[2],"Friendly Fire");
	strcpy(mConfigInfo[2],"Enable or disable friendly fire in Singleplayer");

	mGuiControllers[3] = new JGuiController(3, this, JGUI_STYLE_LEFTRIGHT);
	mGuiControllers[3]->Add(new MenuItem(ANALOG, gFont, "Analog", x, 89, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[3]->Add(new MenuItem(DIRPAD, gFont, "Dir Pad", x+100, 89, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[3]->SetActive(false);
	strcpy(mConfigs[3],"Menu Style");
	strcpy(mConfigInfo[3],"Change the style of the in-game team select and buy menu");

	mGuiControllers[4] = new JGuiController(4, this, JGUI_STYLE_LEFTRIGHT);
	// Respawn selects the location used only when Infection converts a CT into a Zombie.
	mGuiControllers[4]->Add(new MenuItem(RESPAWN_INPLACE, gFont, "In-Place", x, 106, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[4]->Add(new MenuItem(RESPAWN_BASE, gFont, "Base", x+100, 106, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[4]->SetActive(false);
	strcpy(mConfigs[4],"Infection Respawn");
	strcpy(mConfigInfo[4],"Choose where infected players respawn");

	mGuiControllers[5] = new JGuiController(5, this, JGUI_STYLE_LEFTRIGHT);
	// Keep the delay as a numeric menu value so it can be saved and loaded directly.
	mGuiControllers[5]->Add(new MenuItem(INFECTION_DELAY_0, gFont, "0", x, 123, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[5]->Add(new MenuItem(INFECTION_DELAY_1, gFont, "1", x+35, 123, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[5]->Add(new MenuItem(INFECTION_DELAY_2, gFont, "2", x+70, 123, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[5]->Add(new MenuItem(INFECTION_DELAY_3, gFont, "3", x+105, 123, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[5]->Add(new MenuItem(INFECTION_DELAY_4, gFont, "4", x+140, 123, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[5]->SetActive(false);
	strcpy(mConfigs[5],"Infection Delay");
	strcpy(mConfigInfo[5],"Choose the infection respawn delay.");

	mGuiControllers[6] = new JGuiController(6, this, JGUI_STYLE_LEFTRIGHT);
	mGuiControllers[6]->Add(new MenuItem(FREEZE_TIME_0, gFont, "0", x, 140, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[6]->Add(new MenuItem(FREEZE_TIME_1, gFont, "1", x+35, 140, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[6]->Add(new MenuItem(FREEZE_TIME_2, gFont, "2", x+70, 140, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[6]->Add(new MenuItem(FREEZE_TIME_3, gFont, "3", x+105, 140, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[6]->Add(new MenuItem(FREEZE_TIME_4, gFont, "4", x+140, 140, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[6]->Add(new MenuItem(FREEZE_TIME_5, gFont, "5", x+175, 140, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[6]->SetActive(false);
	strcpy(mConfigs[6],"Freeze Time");
	strcpy(mConfigInfo[6],"Choose the delay before each local round starts.");

	mGuiControllers[7] = new JGuiController(7, this, JGUI_STYLE_LEFTRIGHT);
	mGuiControllers[7]->Add(new MenuItem(HORDE_REGROUP_INPLACE, gFont, "In-Place", x, 157, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[7]->Add(new MenuItem(HORDE_REGROUP_BASE, gFont, "CT Base", x+100, 157, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[7]->SetActive(false);
	strcpy(mConfigs[7],"Horde Regroup");
	strcpy(mConfigInfo[7],"Keep survivors in place or return them to CT base.");

	mGuiControllers[8] = new JGuiController(8, this, JGUI_STYLE_LEFTRIGHT);
	mGuiControllers[8]->Add(new MenuItem(HORDE_DELAY_3, gFont, "3", x, 174, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[8]->Add(new MenuItem(HORDE_DELAY_5, gFont, "5", x+35, 174, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[8]->Add(new MenuItem(HORDE_DELAY_10, gFont, "10", x+70, 174, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[8]->Add(new MenuItem(HORDE_DELAY_15, gFont, "15", x+115, 174, TYPE_OPTION, JGETEXT_LEFT));
	mGuiControllers[8]->SetActive(false);
	strcpy(mConfigs[8],"Wave Delay");
	strcpy(mConfigInfo[8],"Choose the freeze time between Horde waves.");

	strcpy(mConfigs[9],"Nickname");
	strcpy(mConfigInfo[9],"Choose a name (15 characters max)");

	strcpy(mConfigs[NUMCONFIGS-1],"Save");
	strcpy(mConfigInfo[NUMCONFIGS-1],"Save any changes and return to the main menu");
	//strcpy(mConfigs[NUMCONFIGS-1],"cancel");
	//strcpy(mConfigInfo[NUMCONFIGS-1],"Ignore any changes and return to the main menu");

	mControlsTex = mRenderer->LoadTexture("gfx/controls.png");
	mControlsQuad = new JQuad(mControlsTex,0,0,480,272);
}

void GameStateOptions::Destroy() 
{
	SAFE_DELETE(mControlsQuad);

	SAFE_DELETE(mControlsTex);

	for (int i=0; i<NUMCONFIGS; i++) {
		SAFE_DELETE(mGuiControllers[i])
	}
}

void GameStateOptions::Start()
{
	strcpy(name,"");

	mRenderer->EnableVSync(true);

	movementstyle = ABSOLUTE1;
	char* movement = GetConfig("data/config.txt","movement");
	if (movement != NULL) {
		if (strcmp(movement,"relative") == 0) {
			movementstyle = RELATIVE1;
		}
		else if (strcmp(movement,"absolute") == 0) {
			movementstyle = ABSOLUTE1;
		}
		delete movement;
	}
	mGuiControllers[0]->SetCurr(movementstyle);

	music = OFF;
	char* musicconfig = GetConfig("data/config.txt","music");
	if (musicconfig != NULL) {
		if (strcmp(musicconfig,"on") == 0) {
			music = ON;
		}
		else if (strcmp(musicconfig,"off") == 0) {
			music = OFF;
		}
		delete musicconfig;
	}
	mGuiControllers[1]->SetCurr(music);

	friendlyfire = ON;
	char* ff = GetConfig("data/config.txt","ff");
	if (ff != NULL) {
		if (strcmp(ff,"on") == 0) {
			friendlyfire = ON;
		}
		else if (strcmp(ff,"off") == 0) {
			friendlyfire = OFF;
		}
		delete ff;
	}
	mGuiControllers[2]->SetCurr(friendlyfire);

	menustyle = ANALOG;
	char* menu = GetConfig("data/config.txt","menu");
	if (menu != NULL) {
		if (strcmp(menu,"analog") == 0) {
			menustyle = ANALOG;
		}
		else if (strcmp(menu,"dirpad") == 0) {
			menustyle = DIRPAD;
		}
		delete menu;
	}
	mGuiControllers[3]->SetCurr(menustyle);

	respawnstyle = RESPAWN_BASE;
	char* respawn = GetConfig("data/config.txt","respawn");
	if (respawn != NULL) {
		if (strcmp(respawn,"inplace") == 0) {
			respawnstyle = RESPAWN_INPLACE;
		}
		else if (strcmp(respawn,"base") == 0) {
			respawnstyle = RESPAWN_BASE;
		}
		delete respawn;
	}
	mGuiControllers[4]->SetCurr(respawnstyle);

	infectiondelay = INFECTION_DELAY_1;
	char* infectionDelay = GetConfig("data/config.txt","infection_respawn_delay");
	if (infectionDelay != NULL) {
		infectiondelay = atoi(infectionDelay);
		if (infectiondelay < INFECTION_DELAY_0 || infectiondelay > INFECTION_DELAY_4) {
			infectiondelay = INFECTION_DELAY_1;
		}
		delete infectionDelay;
	}
	mGuiControllers[5]->SetCurr(infectiondelay);

	freezetime = FREEZE_TIME_3;
	char* freezeTime = GetConfig("data/config.txt","round_freeze_time");
	if (freezeTime != NULL) {
		freezetime = atoi(freezeTime);
		if (freezetime < FREEZE_TIME_0 || freezetime > FREEZE_TIME_5) {
			freezetime = FREEZE_TIME_3;
		}
		delete freezeTime;
	}
	mGuiControllers[6]->SetCurr(freezetime);

	horderegroup = HORDE_REGROUP_BASE;
	char* hordeRegroup = GetConfig("data/config.txt","horde_regroup");
	if (hordeRegroup != NULL) {
		if (strcmp(hordeRegroup,"inplace") == 0) horderegroup = HORDE_REGROUP_INPLACE;
		else if (strcmp(hordeRegroup,"base") == 0) horderegroup = HORDE_REGROUP_BASE;
		delete hordeRegroup;
	}
	mGuiControllers[7]->SetCurr(horderegroup);

	hordewavedelay = HORDE_DELAY_3;
	char* hordeWaveDelay = GetConfig("data/config.txt","horde_wave_delay");
	if (hordeWaveDelay != NULL) {
		int seconds = atoi(hordeWaveDelay);
		if (seconds == 5) hordewavedelay = HORDE_DELAY_5;
		else if (seconds == 10) hordewavedelay = HORDE_DELAY_10;
		else if (seconds == 15) hordewavedelay = HORDE_DELAY_15;
		delete hordeWaveDelay;
	}
	mGuiControllers[8]->SetCurr(hordewavedelay);

	char* nameconfig = GetConfig("data/config.txt","name");
	if (nameconfig != NULL) {
		strncpy(name,nameconfig,15);
		delete nameconfig;
	}
	else {
		strcpy(name,"default");
	}
	name[15] = '\0';

	index = 0;

	mInfoX = -400.0f;
}


void GameStateOptions::End()
{
	mRenderer->EnableVSync(false);
	//delete mGuiControllers[0];
	//mGuiControllers[0] = NULL;
}


void GameStateOptions::Update(float dt)
{
	if (!gDanzeff->mIsActive) {
		if (mEngine->GetButtonClick(PSP_CTRL_LTRIGGER)) {
			mState--;
			if (mState < 0) {
				mState = CONTROLS;
			}
		}
		if (mEngine->GetButtonClick(PSP_CTRL_RTRIGGER)) {
			mState++;
			if (mState > CONTROLS) {
				mState = 0;
			}
		}
		if (mEngine->GetButtonState(PSP_CTRL_RTRIGGER) || mEngine->GetButtonState(PSP_CTRL_LTRIGGER))  {
			for (int i=0; i<NUMCONFIGS; i++) {
				if (mGuiControllers[i] != NULL) {
					mGuiControllers[i]->SetActive(false);
				}
			}
			mGuiControllers[0]->SetCurr(movementstyle);
			mGuiControllers[1]->SetCurr(music);
			mGuiControllers[2]->SetCurr(friendlyfire);
			mGuiControllers[3]->SetCurr(menustyle);
			mGuiControllers[5]->SetCurr(infectiondelay);
			mGuiControllers[4]->SetCurr(respawnstyle);
			mGuiControllers[6]->SetCurr(freezetime);
			mGuiControllers[7]->SetCurr(horderegroup);
			mGuiControllers[8]->SetCurr(hordewavedelay);

			main = true;
			mInfoX = -400.0f;
		}

	}

	if (mState == OPTIONS) {
		mInfoX *= 0.75f;
		if (!gDanzeff->mIsActive) {
			if (main) {
				int temp = index;
				if (mEngine->GetButtonState(PSP_CTRL_UP) || mEngine->GetAnalogY()<64) {
					if (KeyRepeated(PSP_CTRL_UP,dt)) {
						index--;
						if (index < 0) {
							index = NUMCONFIGS-1;
						}
					}
				}
				else if (mEngine->GetButtonState(PSP_CTRL_DOWN) || mEngine->GetAnalogY()>192) {
					if (KeyRepeated(PSP_CTRL_DOWN,dt)) {
						index++;
						if (index > NUMCONFIGS-1) {
							index = 0;
						}
					}
				}
				else if (mEngine->GetButtonClick(PSP_CTRL_RIGHT)) {
					//index = NUMCONFIGS-1;
				}
				else if (mEngine->GetButtonClick(PSP_CTRL_LEFT)) {
					//index = 0;
				}
				else {
					gLastKey = 0;
				}
				if (temp != index) {
					mInfoX = -400.0f;
				}

				if (mEngine->GetButtonClick(PSP_CTRL_CROSS)) {
					if (index == 9) {
						main = false;
						gDanzeff->Enable();
						gDanzeff->mString = name;
						tempname = name;
					}
					else if (index == NUMCONFIGS-1) {
						Save();
						mParent->SetNextState(GAME_STATE_MENU);
					}
					/*else if (index == NUMCONFIGS-1) {
						mParent->SetNextState(GAME_STATE_MENU);
					}*/
					if (mGuiControllers[index] != NULL) {
						//mGuiControllers[index]->SetActive(true);
						main = false;
						mGuiControllers[index]->SetActive(true);
					}
				}
				if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) {
					mParent->SetNextState(GAME_STATE_MENU);
				}
			}
			else {
				if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) {
					for (int i=0; i<NUMCONFIGS; i++) {
						if (mGuiControllers[i] != NULL) {
							mGuiControllers[i]->SetActive(false);
						}
					}
					mGuiControllers[0]->SetCurr(movementstyle);
					mGuiControllers[1]->SetCurr(music);
					mGuiControllers[2]->SetCurr(friendlyfire);
					mGuiControllers[3]->SetCurr(menustyle);
					mGuiControllers[4]->SetCurr(respawnstyle);
					mGuiControllers[5]->SetCurr(infectiondelay);
					mGuiControllers[6]->SetCurr(freezetime);
					mGuiControllers[7]->SetCurr(horderegroup);
					mGuiControllers[8]->SetCurr(hordewavedelay);

					main = true;
				}
			}

			for (int i=0; i<NUMCONFIGS; i++) {
				if (mGuiControllers[i] != NULL) {
					if (mGuiControllers[i]->IsActive()) {
						mGuiControllers[i]->Update(dt);
					}
				}
			}

			/**if (mEngine->GetButtonClick(PSP_CTRL_CROSS)) {
				mParent->SetNextState(GAME_STATE_MENU);
			}
			if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) {
				mParent->SetNextState(GAME_STATE_MENU);
			}**/
		}
		else if (gDanzeff->mIsActive) {
			gDanzeff->Update(dt);
			if (gDanzeff->mString.length() > 15) {
				gDanzeff->mString = gDanzeff->mString.substr(0,15);
			}
			tempname = (char*)gDanzeff->mString.c_str();
			
			if (mEngine->GetButtonClick(PSP_CTRL_START)) {
				main = true;

				bool valid = true;
				FILE* file = fopen("nataku92.txt","r");
				if (file == NULL) {
					if (stricmp(tempname,"nataku92") == 0 || strstr(tempname,"nataku92") != NULL) {
						valid = false;
					}
				}
				else {
					fclose(file);
				}

				if (valid) {
					strcpy(name,tempname);
				}
				else {
					if (stricmp(name,"nataku92") == 0 || strstr(name,"nataku92") != NULL) {
						strcpy(name,"default");
					}
				}
				gDanzeff->Disable();
			}
			else if (mEngine->GetButtonClick(PSP_CTRL_SELECT)) {
				main = true;
				gDanzeff->Disable();
			}
		}
	}
	else if (mState == CONTROLS) {
		if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) {
			mParent->SetNextState(GAME_STATE_MENU);
		}
	}
}


void GameStateOptions::Render()
{
	mRenderer->ClearScreen(ARGB(255,255,255,255));
	//mRenderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(255,255,255,255));
	mRenderer->RenderQuad(gBgQuad, 0.0f, 0.0f);

	gFont->SetBase(0);
	gFont->SetScale(0.75f);
	gFont->SetColor(ARGB(255,255,255,255));
	gFont->DrawShadowedString("< [L]",10,10,JGETEXT_LEFT);
	gFont->DrawShadowedString("[R] >",SCREEN_WIDTH-10,10,JGETEXT_RIGHT);
	gFont->SetScale(1.0f);
	
	if (mState == OPTIONS) {
		mRenderer->FillRect(0,30,SCREEN_WIDTH,5,ARGB(100,0,0,0));
		mRenderer->FillRect(0,35,SCREEN_WIDTH,205,ARGB(100,0,0,0));
		mRenderer->FillRect(0,240,SCREEN_WIDTH,25,ARGB(175,0,0,0));

		mRenderer->FillRect(155-62,5,125,25,ARGB(100,0,0,0));
		mRenderer->FillRect(325-62,5,125,25,ARGB(175,0,0,0));

		gFont->SetColor(ARGB(255,255,255,255));
		gFont->DrawShadowedString("Settings",155,10,JGETEXT_CENTER);
		gFont->SetColor(ARGB(255,175,175,175));
		gFont->DrawShadowedString("Controls",325,10,JGETEXT_CENTER);

		/*gFont->SetColor(ARGB(255,255,255,0));
		gFont->SetScale(1.0f);
		gFont->DrawString("OPTIONS",155,10-2,JGETEXT_CENTER);

		gFont->SetColor(ARGB(255,255,255,255));
		gFont->SetScale(0.75f);
		gFont->DrawString("CONTROLS",325,10,JGETEXT_CENTER);*/

		gFont->SetScale(0.75f);
		gFont->SetColor(ARGB(255,255,255,255));

		if (main) {
			mRenderer->FillRect(0,35+index*17,SCREEN_WIDTH,17,ARGB(255,0,0,0));
			gFont->DrawShadowedString("[X] Select     [O] Cancel and Return to Menu",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
		}
		else {
			mRenderer->FillRect(0,35+index*17,SCREEN_WIDTH,17,ARGB(100,255,128,0));
			if (!gDanzeff->mIsActive) {
				gFont->DrawShadowedString("[DIR PAD/ANALOG] Change Selection    [X] Select    [O] Cancel",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
			}
			else if (gDanzeff->mIsActive) {
				gFont->DrawShadowedString("[START] Enter    [SELECT] Cancel",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
			}
		}
		mRenderer->DrawLine(160,35,160,240,ARGB(255,255,255,255));


		gFont->SetScale(0.75f);
		float x = 160-10;
		for (int i=0; i<NUMCONFIGS; i++) {
			if (index == i) {
				gFont->SetColor(ARGB(255,255,255,255));
				gFont->SetScale(0.75f);
				gFont->DrawShadowedString(mConfigInfo[i],10.0f+mInfoX,225);
				gFont->SetColor(ARGB(255,255,128,0));
			}
			else {
				gFont->SetColor(ARGB(255,255,255,255));
			}
			gFont->SetScale(0.75f);
			gFont->DrawShadowedString(mConfigs[i],x,38+17*i,JGETEXT_RIGHT);
			if (mGuiControllers[i] != NULL) {
				mGuiControllers[i]->Render();
			}

		}
		/*gFont->DrawShadowedString("movement style",x,35,JGETEXT_RIGHT);
		gFont->DrawShadowedString("music",x,60);
		gFont->DrawShadowedString("friendly fire",x,85);
		gFont->DrawShadowedString("name",x,110);
		gFont->DrawShadowedString("save",x,35+(NUMCONFIGS-2)*25);
		gFont->DrawShadowedString("cancel",x,35+(NUMCONFIGS-1)*25);

		gFont->SetBase(128);
		for (int i=0; i<NUMCONFIGS; i++) {
			if (mGuiControllers[i] != NULL) {
				mGuiControllers[i]->Render();
			}
		}*/

		gFont->SetColor(ARGB(255,255,255,255));
		gFont->SetScale(0.75f);
		if (!gDanzeff->mIsActive) {
			gFont->DrawShadowedString(name,160+10,191);
		}
		else if (gDanzeff->mIsActive) {
			gFont->DrawShadowedString(tempname,160+10,191);
			gFont->DrawShadowedString("|",160+10+gFont->GetStringWidth(tempname),191);
			gDanzeff->Render(SCREEN_WIDTH-175,SCREEN_HEIGHT-175);
		}
	} 
	else if (mState == CONTROLS) {
		mRenderer->FillRect(0,30,SCREEN_WIDTH,5,ARGB(100,0,0,0));
		mRenderer->FillRect(0,35,SCREEN_WIDTH,205,ARGB(100,0,0,0));

		mRenderer->FillRect(155-62,5,125,25,ARGB(175,0,0,0));
		mRenderer->FillRect(325-62,5,125,25,ARGB(100,0,0,0));
		mRenderer->RenderQuad(mControlsQuad, 0.0f, 0.0f);


		gFont->SetColor(ARGB(255,255,255,255));
		gFont->DrawShadowedString("Controls",325,10,JGETEXT_CENTER);
		gFont->SetColor(ARGB(255,175,175,175));
		gFont->DrawShadowedString("Settings",155,10,JGETEXT_CENTER);

		/*gFont->SetColor(ARGB(255,255,255,0));
		gFont->SetScale(1.0f);
		gFont->DrawString("CONTROLS",325,10-2,JGETEXT_CENTER);

		gFont->SetColor(ARGB(255,255,255,255));
		gFont->SetScale(0.75f);
		gFont->DrawString("OPTIONS",155,10,JGETEXT_CENTER);*/

		gFont->SetScale(0.75f);
		gFont->SetColor(ARGB(255,255,255,255));
		gFont->DrawShadowedString("[O] Cancel and Return to Menu",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
	}
}

void GameStateOptions::Save()
{
	FILE *file;
	file = fopen("data/config.txt", "w"); 
	if (file == NULL) return;
	
	if (movementstyle == RELATIVE1) {
		fputs("movement = relative\r\n",file);
	}
	else if (movementstyle == ABSOLUTE1) {
		fputs("movement = absolute\r\n",file);	
	}

	if (music == ON) {
		fputs("music = on\r\n",file);
	}
	else if (music == OFF) {
		fputs("music = off\r\n",file);	
	}

	if (friendlyfire == ON) {
		fputs("ff = on\r\n",file);
	}
	else if (friendlyfire == OFF) {
		fputs("ff = off\r\n",file);	
	}

	if (menustyle == ANALOG) {
		fputs("menu = analog\r\n",file);
	}
	else if (menustyle == DIRPAD) {
		fputs("menu = dirpad\r\n",file);	
	}

	if (respawnstyle == RESPAWN_INPLACE) {
		fputs("respawn = inplace\r\n",file);
	}
	else if (respawnstyle == RESPAWN_BASE) {
		fputs("respawn = base\r\n",file);
	}

	fputs("infection_respawn_delay = ",file);
	fprintf(file,"%d",infectiondelay);
	fputs("\r\n",file);

	fputs("round_freeze_time = ",file);
	fprintf(file,"%d",freezetime);
	fputs("\r\n",file);

	if (horderegroup == HORDE_REGROUP_INPLACE) fputs("horde_regroup = inplace\r\n",file);
	else fputs("horde_regroup = base\r\n",file);

	const int hordeWaveDelays[] = {3,5,10,15};
	fputs("horde_wave_delay = ",file);
	fprintf(file,"%d",hordeWaveDelays[hordewavedelay]);
	fputs("\r\n",file);

	fputs("name = ",file);
	fputs(name,file);
	fputs("\r\n",file);

	fputs("map = gunstest\r\n",file);	
	fclose(file);
}

void GameStateOptions::ButtonPressed(int controllerId, int controlId)
{
	main = true;
	mGuiControllers[controllerId]->SetActive(false);
	switch (controllerId)
	{
	case 0:
		movementstyle = controlId;
		//mGuiControllers[0]->mDefault = mGuiControllers[0]->GetCurrentIndex();
		//break;
		break;
	case 1:
		music = controlId;
		//ResetRound();
		//mGuiController->SetActive(false);
		break;
	case 2:
		friendlyfire = controlId;
		//End();
		//mParent->SetNextState(GAME_STATE_MENU);
		break;
	case 3:
		menustyle = controlId;
		//mEngine->End();
		break;
	case 4:
		respawnstyle = controlId;
		break;
	case 5:
		infectiondelay = controlId;
		// Persist the delay immediately so changing this selector cannot be lost.
		Save();
		break;
	case 6:
		freezetime = controlId;
		break;
	case 7:
		horderegroup = controlId;
		break;
	case 8:
		hordewavedelay = controlId;
		break;
	}

	
}

