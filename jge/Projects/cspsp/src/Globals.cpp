#include "Globals.h"

char* gServerName;
char* gServerIP;
int gServerPort;
char* gMapName;
int gTeam;
int gSinglePlayerMode = SINGLEPLAYER_INFECTION;
bool gShowKillFeed = true;
bool gShowRoundTimer = true;
bool gShowHordeTimer = false;
PlayerConfig gPlayerConfig;
GrenadeConfig gGrenadeConfig;
CameraConfig gCameraConfig;
HudConfig gHudConfig;
EffectsConfig gEffectsConfig;
AudioConfig gAudioConfig;
ThemeConfig gThemeConfig;
AimMarkerConfig gAimMarkerConfig;
BotConfig gBotConfig;
LimitsConfig gLimitsConfig;
LaserConfig gLaserConfigs[MAX_GUNS];
ScreenShakeConfig gScreenShakeConfigs[MAX_GUNS];
float gProjectileExplosionRadii[MAX_GUNS];
int gProjectileLaunchOrigins[MAX_GUNS];
JSample* gWeaponExplosionSounds[MAX_GUNS];
JSample* gWeaponImpactSounds[MAX_GUNS];
char gWeaponAnimationProfileNames[MAX_GUNS][64];
KeyFrameAnim* gWeaponAnimationKeyFrames[MAX_GUNS][3];
MuzzlePositionConfig gMuzzlePositionConfigs[MAX_GUNS];
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
JQuad* gPlayersQuads[MAX_PLAYER_SKINS][NUM_QUADS];
JQuad* gPlayersDeadQuads[MAX_PLAYER_SKINS];
bool gPlayerSkinLoaded[MAX_PLAYER_SKINS];
int gPlayerSkinTeams[MAX_PLAYER_SKINS];
char gPlayerSkinNames[MAX_PLAYER_SKINS][PLAYER_SKIN_NAME_LENGTH];
int gTeamSkinIds[2][MAX_PLAYER_SKINS];
int gTeamSkinCounts[2];
char gTeamNames[2][TEAM_NAME_LENGTH];
JQuad* gRadarQuad;
JQuad* gBuyZoneQuad;
JQuad* gDecalQuads[5];
std::map<int, std::vector<JQuad*> > gMuzzleFlashQuads;
JQuad* gBulletImpactQuads[MAX_BULLET_IMPACT_TYPES];
JQuad* gRocketProjectileQuads[MAX_PROJECTILE_STYLES];
std::vector<JQuad*> gRocketExplosionQuads[MAX_EXPLOSION_STYLES];
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

bool IsPlayerSkinValid(int id, int team)
{
	return id >= 0 && id < MAX_PLAYER_SKINS && gPlayerSkinLoaded[id] &&
		(team == NONE || gPlayerSkinTeams[id] == team);
}

int GetDefaultPlayerSkin(int team)
{
	if (team < T || team > CT || gTeamSkinCounts[team] <= 0) return -1;
	return gTeamSkinIds[team][0];
}

int ResolvePlayerSkin(int id, int team)
{
	if (IsPlayerSkinValid(id,team)) return id;
	if (team >= T && team <= CT && id >= 0 && id < gTeamSkinCounts[team]) return gTeamSkinIds[team][id];
	return GetDefaultPlayerSkin(team);
}

int GetRandomPlayerSkin(int team)
{
	if (team < T || team > CT || gTeamSkinCounts[team] <= 0) return -1;
	return gTeamSkinIds[team][rand()%gTeamSkinCounts[team]];
}


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
JSample* gZombieClawsHitSound;
JSample* gDieSounds[2][3];
JSample* gRoundEndSounds[3];
JSample* gHEGrenadeSounds[3];
JSample* gFlashbangSound;
JSample* gSmokeGrenadeSound;
JSample* gGrenadeBounceSound;
JSample* gHitIndicatorSound;

KeyFrameAnim* gKeyFrameAnims[ANIM_COUNT];

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

JQuad* GetMuzzleFlashQuad(int type, int frame)
{
	if (frame < 0 || frame >= MUZZLE_FLASH_FRAMES) frame = 0;
	std::map<int, std::vector<JQuad*> >::iterator found = gMuzzleFlashQuads.find(type);
	if (found != gMuzzleFlashQuads.end() && frame < (int)found->second.size() && found->second[frame] != NULL) return found->second[frame];
	found = gMuzzleFlashQuads.find(0);
	return found != gMuzzleFlashQuads.end() && frame < (int)found->second.size() ? found->second[frame] : NULL;
}

JQuad* GetBulletImpactQuad(int type)
{
	if (type >= 0 && type < MAX_BULLET_IMPACT_TYPES && gBulletImpactQuads[type] != NULL) return gBulletImpactQuads[type];
	return gBulletImpactQuads[0];
}

void LoadLaserConfigs(const char* filename)
{
	for (int i=0; i<MAX_GUNS; i++) {
		gLaserConfigs[i].enabled = false;
		gLaserConfigs[i].falloffEnabled = false;
		gLaserConfigs[i].red = 255;
		gLaserConfigs[i].green = 32;
		gLaserConfigs[i].blue = 24;
		gLaserConfigs[i].alpha = 180;
		gLaserConfigs[i].width = 1.0f;
		gLaserConfigs[i].range = 700.0f;
		gLaserConfigs[i].falloff = 1.0f;
		gLaserConfigs[i].endDot = true;
		gLaserConfigs[i].endDotScale = 1.0f;
		gLaserConfigs[i].offsetX = 12.0f;
		gLaserConfigs[i].offsetY = 0.0f;
	}
	FILE* file = fopen(filename,"r");
	if (file == NULL) return;
	char line[512];
	while (fgets(line,sizeof(line),file) != NULL) {
		if (line[0] == '#') continue;
		int id, enabled, falloffEnabled, red, green, blue, alpha, endDot;
		float width, range, falloff, endDotScale, offsetX = 12.0f, offsetY = 0.0f;
		int fields = sscanf(line,"%d %d %d %d %d %d %d %f %f %f %d %f %f %f",&id,&enabled,&falloffEnabled,
			&red,&green,&blue,&alpha,&width,&range,&falloff,&endDot,&endDotScale,&offsetX,&offsetY);
		if (fields != 12 && fields != 14) continue;
		if (id < 0 || id >= MAX_GUNS) continue;
		LaserConfig& config = gLaserConfigs[id];
		config.enabled = enabled != 0;
		config.falloffEnabled = falloffEnabled != 0;
		config.red = std::max(0,std::min(255,red));
		config.green = std::max(0,std::min(255,green));
		config.blue = std::max(0,std::min(255,blue));
		config.alpha = std::max(0,std::min(255,alpha));
		config.width = std::max(0.1f,std::min(20.0f,width));
		config.range = std::max(1.0f,std::min(5000.0f,range));
		config.falloff = std::max(0.0f,std::min(10.0f,falloff));
		config.endDot = endDot != 0;
		config.endDotScale = std::max(0.1f,std::min(20.0f,endDotScale));
		config.offsetX = std::max(-100.0f,std::min(100.0f,offsetX));
		config.offsetY = std::max(-100.0f,std::min(100.0f,offsetY));
	}
	fclose(file);
}

void LoadScreenShakeConfigs(const char* filename)
{
	for (int i=0; i<MAX_GUNS; i++) {
		gScreenShakeConfigs[i].fireMagnitude = 0;
		gScreenShakeConfigs[i].fireTime = 0.0f;
		gScreenShakeConfigs[i].explosionMagnitude = 0;
		gScreenShakeConfigs[i].explosionTime = 0.0f;
		gScreenShakeConfigs[i].explosionRadius = 0.0f;
	}
	FILE* file = fopen(filename,"r");
	if (file == NULL) return;
	char line[256];
	while (fgets(line,sizeof(line),file) != NULL) {
		if (line[0] == '#') continue;
		int id, fireMagnitude, explosionMagnitude;
		float fireTime, explosionTime, explosionRadius;
		if (sscanf(line,"%d %d %f %d %f %f",&id,&fireMagnitude,&fireTime,&explosionMagnitude,&explosionTime,&explosionRadius) != 6) continue;
		if (id < 0 || id >= MAX_GUNS) continue;
		ScreenShakeConfig& config = gScreenShakeConfigs[id];
		config.fireMagnitude = std::max(0,std::min(100,fireMagnitude));
		config.fireTime = std::max(0.0f,std::min(5000.0f,fireTime));
		config.explosionMagnitude = std::max(0,std::min(100,explosionMagnitude));
		config.explosionTime = std::max(0.0f,std::min(5000.0f,explosionTime));
		config.explosionRadius = std::max(0.0f,std::min(5000.0f,explosionRadius));
	}
	fclose(file);
}

void LoadProjectileExplosionConfigs(const char* filename)
{
	for (int i=0; i<MAX_GUNS; i++) gProjectileExplosionRadii[i] = -1.0f;
	FILE* file = fopen(filename,"r");
	if (file == NULL) return;
	char line[128];
	while (fgets(line,sizeof(line),file) != NULL) {
		if (line[0] == '#') continue;
		int id;
		float radius;
		if (sscanf(line,"%d %f",&id,&radius) != 2) continue;
		if (id < 0 || id >= MAX_GUNS) continue;
		gProjectileExplosionRadii[id] = std::max(0.0f,std::min(5000.0f,radius));
	}
	fclose(file);
}

void LoadProjectileLaunchOrigins(const char* filename)
{
	for (int i=0; i<MAX_GUNS; i++) gProjectileLaunchOrigins[i] = -1;
	FILE* file = fopen(filename,"r");
	if (file == NULL) return;
	char line[128];
	while (fgets(line,sizeof(line),file) != NULL) {
		if (line[0] == '#') continue;
		int id, origin;
		if (sscanf(line,"%d %d",&id,&origin) != 2) continue;
		if (id < 0 || id >= MAX_GUNS) continue;
		gProjectileLaunchOrigins[id] = origin == 1 ? 1 : 0;
	}
	fclose(file);
}

void LoadWeaponAnimationProfiles(const char* filename)
{
	for (int i=0; i<MAX_GUNS; i++) {
		gWeaponAnimationProfileNames[i][0] = '\0';
		for (int action=0; action<3; action++) gWeaponAnimationKeyFrames[i][action] = NULL;
	}
	FILE* file = fopen(filename,"r");
	if (file == NULL) return;
	char line[128];
	while (fgets(line,sizeof(line),file) != NULL) {
		if (line[0] == '#') continue;
		int id;
		char profile[64];
		if (sscanf(line,"%d %63s",&id,profile) != 2) continue;
		if (id < 0 || id >= MAX_GUNS) continue;
		if (strcmp(profile,"0") == 0) continue;
		if (strcmp(profile,"1") == 0) strcpy(profile,"SPECIAL");
		for (int index=0; profile[index] != '\0'; index++) profile[index] = toupper(profile[index]);
		char blockName[80];
		strcpy(blockName,profile);
		if (!Animation::HasKeyFrames(blockName)) continue;
		sprintf(blockName,"%s_FIRE",profile);
		if (!Animation::HasKeyFrames(blockName)) continue;
		sprintf(blockName,"%s_RELOAD",profile);
		if (!Animation::HasKeyFrames(blockName)) continue;
		strcpy(gWeaponAnimationProfileNames[id],profile);
		strcpy(blockName,profile);
		gWeaponAnimationKeyFrames[id][0] = Animation::LoadKeyFrames(blockName);
		sprintf(blockName,"%s_FIRE",profile);
		gWeaponAnimationKeyFrames[id][1] = Animation::LoadKeyFrames(blockName);
		sprintf(blockName,"%s_RELOAD",profile);
		gWeaponAnimationKeyFrames[id][2] = Animation::LoadKeyFrames(blockName);
	}
	fclose(file);
}

void LoadMuzzlePositionConfigs(const char* filename)
{
	for (int i=0; i<MAX_GUNS; i++) {
		gMuzzlePositionConfigs[i].forward = 0.0f;
		gMuzzlePositionConfigs[i].sideways = 0.0f;
	}
	FILE* file = fopen(filename,"r");
	if (file == NULL) return;
	char line[128];
	while (fgets(line,sizeof(line),file) != NULL) {
		if (line[0] == '#') continue;
		int id;
		float forward, sideways;
		if (sscanf(line,"%d %f %f",&id,&forward,&sideways) != 3) continue;
		if (id < 0 || id >= MAX_GUNS) continue;
		gMuzzlePositionConfigs[id].forward = std::max(-100.0f,std::min(100.0f,forward));
		gMuzzlePositionConfigs[id].sideways = std::max(-100.0f,std::min(100.0f,sideways));
	}
	fclose(file);
}

bool SetConfigValue(const char* location, const char* key, const char* value)
{
	FILE* file = fopen(location,"r");
	std::vector<std::string> lines;
	char line[1024];
	bool found = false;
	while (file != NULL && fgets(line,sizeof(line),file) != NULL) {
		char currentKey[64];
		if (sscanf(line,"%63s",currentKey) == 1 && strcmp(currentKey,key) == 0) {
			std::string replacement = key;
			replacement += " = ";
			replacement += value;
			replacement += "\r\n";
			lines.push_back(replacement);
			found = true;
		}
		else {
			lines.push_back(line);
		}
	}
	if (file != NULL) fclose(file);

	if (!found) {
		std::string replacement = key;
		replacement += " = ";
		replacement += value;
		replacement += "\r\n";
		lines.push_back(replacement);
	}

	file = fopen(location,"w");
	if (file == NULL) return false;
	for (unsigned int i=0; i<lines.size(); i++) fputs(lines[i].c_str(),file);
	fclose(file);
	return true;
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

	const char* aimMarkerFile = "data/aimmarkers.txt";
	gAimMarkerConfig.guideColor = LoadConfigColor(aimMarkerFile,"guide_color",gThemeConfig.crosshairColor);
	gAimMarkerConfig.guideInner = LoadConfigFloat(aimMarkerFile,"guide_inner",gHudConfig.crosshairInner,0.0f,500.0f);
	gAimMarkerConfig.guideLength = LoadConfigFloat(aimMarkerFile,"guide_length",gHudConfig.crosshairOuter-gHudConfig.crosshairInner,0.0f,500.0f);
	gAimMarkerConfig.guideWidth = LoadConfigFloat(aimMarkerFile,"guide_width",1.0f,0.1f,20.0f);
	gAimMarkerConfig.hitmarkerColor = LoadConfigColor(aimMarkerFile,"hitmarker_color",gThemeConfig.crosshairHitColor);
	gAimMarkerConfig.hitmarkerInner = LoadConfigFloat(aimMarkerFile,"hitmarker_inner",gHudConfig.crosshairInner,0.0f,500.0f);
	gAimMarkerConfig.hitmarkerLength = LoadConfigFloat(aimMarkerFile,"hitmarker_length",gHudConfig.crosshairOuter+5.0f-gHudConfig.crosshairInner,0.0f,500.0f);
	gAimMarkerConfig.hitmarkerWidth = LoadConfigFloat(aimMarkerFile,"hitmarker_width",1.0f,0.1f,20.0f);

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
	gShowHordeTimer = false;
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
	char* hordeTimer = GetConfig("data/config.txt","show_horde_timer");
	if (hordeTimer != NULL) {
		gShowHordeTimer = strcmp(hordeTimer,"off") != 0;
		delete[] hordeTimer;
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