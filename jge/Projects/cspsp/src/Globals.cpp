#include "Globals.h"

char* gServerName;
char* gServerIP;
int gServerPort;
char* gMapName;
int gTeam;
int gSinglePlayerMode = SINGLEPLAYER_INFECTION;
bool gShowKillFeed = true;
bool gShowRoundTimer = true;
PlayerConfig gPlayerConfig;
GrenadeConfig gGrenadeConfig;
CameraConfig gCameraConfig;
HudConfig gHudConfig;
EffectsConfig gEffectsConfig;
AudioConfig gAudioConfig;
ThemeConfig gThemeConfig;
BotConfig gBotConfig;
LimitsConfig gLimitsConfig;
bool gReconnect;
bool gLogout;
char gName[32];
char gDisplayName[32];
char gEncodedName[100];
char gEncodedDisplayName[100];
char gKey[64];
char gSessionKey[32];
int gKills;
int gDeaths;
int gKills2;
int gDeaths2;
unsigned char gIcon[300];

JLBFont* gFont;
JLBFont* gFontBackdrop;
JLBFont* gHudFont;

JQuad* gBgQuad;
JQuad* gLogoQuad;

Gun gGuns[MAX_GUNS];
JQuad** gGunHandQuads;
JQuad** gGunGroundQuads;
JQuad* gPlayersQuads[2][4][NUM_QUADS];
JQuad* gPlayersDeadQuads[2][4];
JQuad* gRadarQuad;
JQuad* gBuyZoneQuad;
JQuad* gDecalQuads[5];
JQuad* gMuzzleFlashQuads[3];
JQuad* gHealthBorderQuad;
JQuad* gHealthFillQuad;
JQuad* gArmorBorderQuad;
JQuad* gArmorFillQuad;
JQuad* gArmorGroundQuad;
JQuad* gHealthGroundQuad;
JQuad* gAmmoGroundQuad;
JQuad* gHealthPickupQuad;
JQuad* gArmorPickupQuad;
JQuad* gAmmoPickupQuad;
JQuad* gAmmoBarQuad;
JQuad* gIconQuad;
JTexture* gIconTexture;
JQuad* gDamageIndicator;
JQuad* gScoreIconQuads[4];
JQuad* gFlagQuad;
JQuad* gFlagHomeQuad;
JQuad* gFlagArrowQuad;
JQuad* gFlagRadarQuad;
JQuad* gHomeRadarQuad;


JSample* gDryFireRifleSound;
JSample* gDryFirePistolSound;
JSample* gDeploySound;
JSample* gPickUpSound;
JSample* gAmmoSound;
JSample* gPinPullSound;
JSample* gFireInTheHoleSound;
JSample* gWalkSounds[2];
JSample* gRicochetSounds[4];
JSample* gHitSounds[3];
JSample* gKnifeHitSound;
JSample* gDieSounds[3];
JSample* gRoundEndSounds[3];
JSample* gHEGrenadeSounds[3];
JSample* gFlashbangSound;
JSample* gSmokeGrenadeSound;
JSample* gGrenadeBounceSound;
JSample* gHitIndicatorSound;

KeyFrameAnim* gKeyFrameAnims[11];

ParticleEngine* gParticleEngine;
hgeParticleSystem* gParticleSystems[3];
SfxManager* gSfxManager;
Danzeff* gDanzeff;
Socket* gSocket;
HttpManager* gHttpManager;

char* GetConfig(const char *location, char searchstr[]) {
    FILE *file;
	file = fopen(location, "r"); 
	if (file == NULL) return NULL;
	else {
		char line[1024]; 
		// Keep this large enough for newer config keys such as infection_respawn_delay.
		char key[64];
		char value[64];
		char* valueptr = new char[64];	//*************** initialize pointer
		while (fgets(line,1024,file) != NULL) {
			if (sscanf(line,"%63s %*s %63s",key,value) != 2) continue;
			if (strcmp(searchstr,key) == 0) {
				strcpy(valueptr, (char*)value); //*************** something that i don't understand D:
				fclose(file);
				return valueptr;
			}
		}
	}
	fclose(file);
	return NULL;
}

static int LoadConfigInt(const char* file, char* key, int defaultValue, int minimum, int maximum)
{
	char* value = GetConfig(file,key);
	int result = value == NULL ? defaultValue : atoi(value);
	delete[] value;
	if (result < minimum) result = minimum;
	if (result > maximum) result = maximum;
	return result;
}

static float LoadConfigFloat(const char* file, char* key, float defaultValue, float minimum, float maximum)
{
	char* value = GetConfig(file,key);
	float result = value == NULL ? defaultValue : (float)atof(value);
	delete[] value;
	if (result < minimum) result = minimum;
	if (result > maximum) result = maximum;
	return result;
}

static u32 LoadConfigColor(const char* file, char* key, u32 defaultValue)
{
	char* value = GetConfig(file,key);
	u32 result = value == NULL ? defaultValue : (u32)strtoul(value,NULL,16);
	delete[] value;
	return result;
}

void LoadClientDataConfigs()
{
	const char* playerFile = "data/player.txt";
	gPlayerConfig.ctMaxHealth = LoadConfigInt(playerFile,"ct_max_health",100,1,32767);
	gPlayerConfig.tMaxHealth = LoadConfigInt(playerFile,"t_max_health",150,1,32767);
	gPlayerConfig.ctMaxArmor = LoadConfigInt(playerFile,"ct_max_armor",100,0,32767);
	gPlayerConfig.tMaxArmor = LoadConfigInt(playerFile,"t_max_armor",100,0,32767);
	gPlayerConfig.ctSpawnHealth = LoadConfigInt(playerFile,"ct_spawn_health",100,1,gPlayerConfig.ctMaxHealth);
	gPlayerConfig.tSpawnHealth = LoadConfigInt(playerFile,"t_spawn_health",150,1,gPlayerConfig.tMaxHealth);
	gPlayerConfig.ctSpawnArmor = LoadConfigInt(playerFile,"ct_spawn_armor",0,0,gPlayerConfig.ctMaxArmor);
	gPlayerConfig.tSpawnArmor = LoadConfigInt(playerFile,"t_spawn_armor",0,0,gPlayerConfig.tMaxArmor);
	gPlayerConfig.armorDamageReduction = LoadConfigInt(playerFile,"armor_damage_reduction",30,0,100);
	gPlayerConfig.pickupRadius = LoadConfigFloat(playerFile,"pickup_radius",16.0f,1.0f,256.0f);
	gPlayerConfig.startingMoney = LoadConfigInt(playerFile,"starting_money",800,0,32767);
	gPlayerConfig.damageSpeedMultiplier = LoadConfigFloat(playerFile,"damage_speed_multiplier",0.1f,0.0f,1.0f);
	gPlayerConfig.acceleration = LoadConfigFloat(playerFile,"player_acceleration",0.0005f,0.0f,1.0f);
	gPlayerConfig.deceleration = LoadConfigFloat(playerFile,"player_deceleration",0.0005f,0.0f,1.0f);

	const char* grenadeFile = "data/grenades.txt";
	gGrenadeConfig.fuseTime = LoadConfigFloat(grenadeFile,"fuse_time_ms",1500.0f,0.0f,60000.0f);
	gGrenadeConfig.flashDuration = LoadConfigFloat(grenadeFile,"flash_duration_ms",15000.0f,0.0f,60000.0f);
	gGrenadeConfig.flashMinDistance = LoadConfigFloat(grenadeFile,"flash_min_distance",20.0f,1.0f,10000.0f);
	gGrenadeConfig.flashFalloff = LoadConfigFloat(grenadeFile,"flash_falloff",20.0f,0.0f,10000.0f);
	gGrenadeConfig.heMinDistance = LoadConfigFloat(grenadeFile,"he_min_distance",40.0f,1.0f,10000.0f);
	gGrenadeConfig.heMaxDistance = LoadConfigFloat(grenadeFile,"he_max_distance",200.0f,gGrenadeConfig.heMinDistance,10000.0f);
	gGrenadeConfig.heFalloff = LoadConfigFloat(grenadeFile,"he_falloff",40.0f,0.0f,10000.0f);

	const char* cameraFile = "data/camera.txt";
	gCameraConfig.smoothing = LoadConfigFloat(cameraFile,"smoothing",0.005f,0.0001f,1.0f);
	gCameraConfig.lookAheadVelocity = LoadConfigFloat(cameraFile,"look_ahead_velocity",0.5f,0.0f,10.0f);
	gCameraConfig.lookAheadDistance = LoadConfigFloat(cameraFile,"look_ahead_distance",500.0f,0.0f,10000.0f);
	gCameraConfig.scopeNone = LoadConfigInt(cameraFile,"scope_none",500,0,10000);
	gCameraConfig.scopeLow = LoadConfigInt(cameraFile,"scope_low",800,0,10000);
	gCameraConfig.scopeMedium = LoadConfigInt(cameraFile,"scope_medium",1100,0,10000);
	gCameraConfig.scopeHigh = LoadConfigInt(cameraFile,"scope_high",1400,0,10000);
	gCameraConfig.analogDeadzone = LoadConfigInt(cameraFile,"analog_deadzone",20,0,127);
	gCameraConfig.analogMaxSpeed = LoadConfigFloat(cameraFile,"analog_max_speed",0.1f,0.0f,1.0f);
	gCameraConfig.rotationSpeed = LoadConfigFloat(cameraFile,"rotation_speed",0.005f,0.0f,1.0f);

	const char* hudFile = "data/hud.txt";
	gHudConfig.killFeedLifetime = LoadConfigFloat(hudFile,"kill_feed_lifetime_ms",3000.0f,0.0f,60000.0f);
	gHudConfig.messageLifetime = LoadConfigFloat(hudFile,"message_lifetime_ms",5000.0f,0.0f,60000.0f);
	gHudConfig.centerMessageLifetime = LoadConfigFloat(hudFile,"center_message_lifetime_ms",3000.0f,0.0f,60000.0f);
	gHudConfig.popupLifetime = LoadConfigFloat(hudFile,"popup_lifetime_ms",2000.0f,0.0f,60000.0f);
	gHudConfig.damageIndicatorLifetime = LoadConfigFloat(hudFile,"damage_indicator_lifetime_ms",1500.0f,0.0f,60000.0f);
	gHudConfig.killFeedX = LoadConfigFloat(hudFile,"kill_feed_x",475.0f,0.0f,480.0f);
	gHudConfig.lineSpacing = LoadConfigFloat(hudFile,"line_spacing",15.0f,1.0f,100.0f);
	gHudConfig.healthX = LoadConfigFloat(hudFile,"health_x",0.0f,0.0f,480.0f);
	gHudConfig.armorX = LoadConfigFloat(hudFile,"armor_x",26.0f,0.0f,480.0f);
	gHudConfig.barsBottomOffset = LoadConfigFloat(hudFile,"bars_bottom_offset",58.0f,0.0f,272.0f);
	gHudConfig.timerBottomOffset = LoadConfigFloat(hudFile,"timer_bottom_offset",25.0f,0.0f,272.0f);
	gHudConfig.radarX = LoadConfigFloat(hudFile,"radar_x",10.0f,0.0f,480.0f);
	gHudConfig.radarY = LoadConfigFloat(hudFile,"radar_y",10.0f,0.0f,272.0f);
	gHudConfig.radarScale = LoadConfigFloat(hudFile,"radar_scale",0.0666667f,0.001f,10.0f);
	gHudConfig.crosshairInner = LoadConfigFloat(hudFile,"crosshair_inner",30.0f,0.0f,500.0f);
	gHudConfig.crosshairOuter = LoadConfigFloat(hudFile,"crosshair_outer",40.0f,gHudConfig.crosshairInner,500.0f);

	const char* effectsFile = "data/effects.txt";
	gEffectsConfig.maxDecals = LoadConfigInt(effectsFile,"max_decals",100,0,10000);
	gEffectsConfig.particlePoolSize = LoadConfigInt(effectsFile,"particle_pool_size",100,1,10000);
	gEffectsConfig.bloodParticleCount = LoadConfigInt(effectsFile,"blood_particle_count",5,0,1000);
	gEffectsConfig.impactParticleCount = LoadConfigInt(effectsFile,"impact_particle_count",2,0,1000);
	gEffectsConfig.shellParticleCount = LoadConfigInt(effectsFile,"shell_particle_count",1,0,1000);
	gEffectsConfig.bloodParticleLifetime = LoadConfigFloat(effectsFile,"blood_particle_lifetime_ms",200.0f,1.0f,60000.0f);
	gEffectsConfig.impactParticleLifetime = LoadConfigFloat(effectsFile,"impact_particle_lifetime_ms",250.0f,1.0f,60000.0f);
	gEffectsConfig.shellParticleLifetime = LoadConfigFloat(effectsFile,"shell_particle_lifetime_ms",500.0f,1.0f,60000.0f);
	gEffectsConfig.muzzleFlashLifetime = LoadConfigFloat(effectsFile,"muzzle_flash_lifetime_ms",50.0f,0.0f,10000.0f);

	const char* audioFile = "data/audio.txt";
	gAudioConfig.globalVolume = LoadConfigInt(audioFile,"global_volume",256,0,256);
	gAudioConfig.positionalDistance = LoadConfigFloat(audioFile,"positional_distance",500.0f,1.0f,100000.0f);
	gAudioConfig.panningDeadzone = LoadConfigFloat(audioFile,"panning_deadzone",5.0f,0.0f,10000.0f);

	const char* themeFile = "data/theme.txt";
	gThemeConfig.ctColor = LoadConfigColor(themeFile,"ct_color",ARGB(255,153,204,255));
	gThemeConfig.tColor = LoadConfigColor(themeFile,"t_color",ARGB(255,255,64,64));
	gThemeConfig.crosshairColor = LoadConfigColor(themeFile,"crosshair_color",ARGB(200,0,255,0));
	gThemeConfig.crosshairHitColor = LoadConfigColor(themeFile,"crosshair_hit_color",ARGB(255,255,0,0));
	gThemeConfig.radarFriendlyColor = LoadConfigColor(themeFile,"radar_friendly_color",ARGB(255,0,255,0));
	gThemeConfig.radarEnemyColor = LoadConfigColor(themeFile,"radar_enemy_color",ARGB(255,255,0,0));

	const char* botFile = "data/bots.txt";
	gBotConfig.visionRange = LoadConfigFloat(botFile,"vision_range",223.6068f,1.0f,10000.0f);
	gBotConfig.reactionTime = LoadConfigFloat(botFile,"reaction_time_ms",100.0f,1.0f,60000.0f);
	gBotConfig.stuckTime = LoadConfigFloat(botFile,"stuck_time_ms",3000.0f,1.0f,60000.0f);
	gBotConfig.waypointTolerance = LoadConfigFloat(botFile,"waypoint_tolerance",31.6228f,1.0f,10000.0f);
	gBotConfig.pathTolerance = LoadConfigFloat(botFile,"path_tolerance",22.3607f,1.0f,10000.0f);
	gBotConfig.rotationSpeed = LoadConfigFloat(botFile,"rotation_speed",0.003f,0.0f,1.0f);
	gBotConfig.movementSpeed = LoadConfigFloat(botFile,"movement_speed",0.08f,0.0f,10.0f);
	gBotConfig.fireDelayMin = LoadConfigInt(botFile,"fire_delay_min_ms",250,0,60000);
	gBotConfig.fireDelayMax = LoadConfigInt(botFile,"fire_delay_max_ms",750,gBotConfig.fireDelayMin,60000);
	gBotConfig.burstMin = LoadConfigInt(botFile,"burst_min_ms",500,0,60000);
	gBotConfig.burstMax = LoadConfigInt(botFile,"burst_max_ms",1500,gBotConfig.burstMin,60000);

	const char* limitsFile = "data/limits.txt";
	gLimitsConfig.killFeedEvents = LoadConfigInt(limitsFile,"kill_feed_events",5,1,100);
	gLimitsConfig.messageEvents = LoadConfigInt(limitsFile,"message_events",9,1,100);
	gLimitsConfig.activeMessageEvents = LoadConfigInt(limitsFile,"active_message_events",3,1,gLimitsConfig.messageEvents);
}

void LoadHudDisplayOptions()
{
	gShowKillFeed = true;
	gShowRoundTimer = true;
	char* killFeed = GetConfig("data/config.txt","show_kill_feed");
	if (killFeed != NULL) {
		gShowKillFeed = strcmp(killFeed,"off") != 0;
		delete[] killFeed;
	}
	char* roundTimer = GetConfig("data/config.txt","show_round_timer");
	if (roundTimer != NULL) {
		gShowRoundTimer = strcmp(roundTimer,"off") != 0;
		delete[] roundTimer;
	}
}

int GetArmorDamageReduction()
{
	return gPlayerConfig.armorDamageReduction;
}

u32 gLastKey;
float gKeyRepeatDelay;
bool KeyRepeated(u32 key, float dt)
{

	bool doKey = false;
	if (gLastKey != PSP_CTRL_UP)
	{
		gLastKey = PSP_CTRL_UP;
		doKey = true;
		gKeyRepeatDelay = JGUI_INITIAL_DELAY;
	}
	else
	{
		gKeyRepeatDelay -= dt;
		if (gKeyRepeatDelay <= 0.0f)
		{
			gKeyRepeatDelay = 50;//JGUI_REPEAT_DELAY;
			doKey = true;
		}
	}

	return doKey;
}

char gBuffer[8192];
bool ReadHTTP(char* string) {
	char* s;

	if (strstr(string,"chunked")) {
		s = strstr(string,"\r\n\r\n");
		s += 4;
	}
	else {
		s = string;
	}
	
	while (s) {
		int length = strtol(s,&s,16);
		if (length == 0) {
			return true;
		}
		s += 2; // \r\n

		strncat(gBuffer,&s[0],length);
		s += length;
	}

	return false;
}


void FormatText(std::vector<char*> &lines, char text[], float width, float scale) {
	gFont->SetScale(scale);

	string line = "";
	string word = "";

	for (int i=0; i<strlen(text); i++) {
		if (text[i] == '\r' || text[i] == '\n') {
			if (text[i] == '\r') i++;
			if (gFont->GetStringWidth(line.c_str()) + gFont->GetStringWidth(word.c_str()) + gFont->GetStringWidth(" ") > width) {
				char* buffer = new char[256];
				strcpy(buffer,line.c_str());
				lines.push_back(buffer);
				buffer = new char[256];
				strcpy(buffer,word.c_str());
				lines.push_back(buffer);
				line = "";
				word = "";
			}
			else {
				line += word + " ";
				char* buffer = new char[256];
				strcpy(buffer,line.c_str());
				lines.push_back(buffer);
				line = "";
				word = "";
			}
		}
		else if (text[i] == ' ') {
			if (gFont->GetStringWidth(line.c_str()) + gFont->GetStringWidth(word.c_str()) + gFont->GetStringWidth(" ") > width) {
				char* buffer = new char[256];
				strcpy(buffer,line.c_str());
				lines.push_back(buffer);
				line = "";
				line += word + " ";
				word = "";
			}
			else {
				line += word + " ";
				word = "";
			}
		}
		else {
			if (gFont->GetStringWidth((word+text[i]).c_str()) > width) {
				char* buffer = new char[256];
				strcpy(buffer,word.c_str());
				lines.push_back(buffer);
				line = "";
				word = "";
			}
			word += text[i];
		}
	}
	if (gFont->GetStringWidth(line.c_str()) + gFont->GetStringWidth(word.c_str()) + gFont->GetStringWidth(" ") > width) {
		char* buffer = new char[256];
		strcpy(buffer,line.c_str());
		lines.push_back(buffer);
		line = "";
		buffer = new char[256];
		strcpy(buffer,word.c_str());
		lines.push_back(buffer);
	}
	else {
		line += word;
		char* buffer = new char[256];
		strcpy(buffer,line.c_str());
		lines.push_back(buffer);
		line = "";
	}

	//return lines;
}

void FormatText(char* buffer, char text[], float width, float scale) {
	gFont->SetScale(scale);

	strcpy(buffer,"");

	for (int i=0; i<strlen(text); i++) {
		buffer[i] = text[i];
		buffer[i+1] = '\0';
		if (gFont->GetStringWidth(buffer) >= width) {
			buffer[i] = '\0';
			break;
		}
	}
}

char* DecodeText(char* buffer, char* text) {
	//char buffer[2000];
	strcpy(buffer,"");
	for (int i=0; i<strlen(text); i+=3) {
		int factor = 100;
		if ((i/3)%2 == 0) factor = 101;

		char numberBuffer[4];
		strcpy(numberBuffer,"");
		numberBuffer[0] = text[i];
		numberBuffer[1] = text[i+1];
		numberBuffer[2] = text[i+2];
		numberBuffer[3] = '\0';

		int number = 0;
		sscanf(numberBuffer,"%d",&number);
		number -= factor;

		sprintf(numberBuffer,"%c",number);
		strcat(buffer,numberBuffer);
	}

	return buffer;
}

void EncodeText(char* dest, char* text) {
	strcpy(dest,"");
	for (int i=0; i<strlen(text); i++) {
		int factor = 100;
		if (i%2 == 0) factor = 101;

		char numberBuffer[4];
		int number = text[i];

		sprintf(numberBuffer,"%d",number+factor);

		strcat(dest,numberBuffer);
	}
	//return buffer;
}

void DrawShadowedString(const char *string, float x, float y, int align) {
	PIXEL_TYPE color = gFont->GetColor();
	gFont->SetColor(ARGB(200,0,0,0));
	gFont->DrawString(string,x+1,y+1,align);

	/*gFontBackdrop->SetColor(ARGB(200,0,0,0));
	gFontBackdrop->SetScale(gFont->GetScale());
	gFontBackdrop->DrawString(string,x,y,align);*/

	gFont->SetColor(color);
	gFont->DrawString(string,x,y,align);
}

void UpdateIcon(JTexture* texture, unsigned char* data) {
	DWORD bits[100];
	int i=0;
	for (int y=0; y<10; y++) {
		for (int x=0; x<10; x++) {
			int a = 255;
			if (data[i] == 255 && data[i+2] == 255 && data[i+1] == 0) {
				a = 0;
			}
			bits[(9-y)*10+x] = ARGB(a,data[i+2],data[i+1],data[i]);
			i += 3;
		}
	}
	texture->UpdateBits(0,0,10,10,bits);
}