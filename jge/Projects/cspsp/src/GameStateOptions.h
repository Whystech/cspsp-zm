#ifndef _GAME_STATE_OPTIONS_H_
#define _GAME_STATE_OPTIONS_H_

#include "JGui.h"
#include "GameState.h"
#include "MenuItem.h"
#include <vector>

#ifdef WIN32
#else
#include <psputility.h>
#endif

#define OPTIONS 0
#define CONTROLS 1

#define NUMCONFIGS 13

#define RELATIVE1 0
#define ABSOLUTE1 1

#define ON 0
#define OFF 1

#define ANALOG 0
#define DIRPAD 1

#define RESPAWN_INPLACE 0
#define RESPAWN_BASE 1

#define INFECTION_DELAY_0 0
#define INFECTION_DELAY_1 1
#define INFECTION_DELAY_2 2
#define INFECTION_DELAY_3 3
#define INFECTION_DELAY_4 4

#define FREEZE_TIME_0 0
#define FREEZE_TIME_1 1
#define FREEZE_TIME_2 2
#define FREEZE_TIME_3 3
#define FREEZE_TIME_4 4
#define FREEZE_TIME_5 5

#define HORDE_REGROUP_INPLACE 0
#define HORDE_REGROUP_BASE 1

#define HORDE_DELAY_3 0
#define HORDE_DELAY_5 1
#define HORDE_DELAY_10 2
#define HORDE_DELAY_15 3

#define SAVE 12
#define CANCEL 13

class GameStateOptions:	public GameState,
	 					public JGuiListener
{
private:
	JGuiController* mGuiControllers[NUMCONFIGS];
	char mConfigs[NUMCONFIGS][256];
	char mConfigInfo[NUMCONFIGS][256];

	int mState;
	int index;
	bool main;
	int movementstyle;
	int music;
	int friendlyfire;
	int menustyle;
	int respawnstyle;
	int infectiondelay;
	int freezetime;
	int horderegroup;
	int hordewavedelay;
	int showkillfeed;
	int showroundtimer;
	char name[16];
	char* tempname;
	JTexture* mControlsTex;
	JQuad* mControlsQuad;

	float mInfoX;

public:
	GameStateOptions(GameApp* parent);
	~GameStateOptions();

	void Create();
	void Destroy();
	void Start();
	void End();
	void Update(float dt);
	void Render();

	void ButtonPressed(int controllerId, int controlId);

	void Save();
};


#endif

