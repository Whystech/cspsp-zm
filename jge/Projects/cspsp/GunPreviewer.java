import java.awt.* ;
import java.awt.event.* ;
import java.awt.geom.AffineTransform ;
import java.awt.geom.Point2D ;
import java.awt.image.BufferedImage ;
import java.awt.image.RescaleOp ;
import java.io.* ;
import java.net.URISyntaxException ;
import java.util.ArrayList ;
import java.util.HashMap ;
import java.util.LinkedHashMap ;
import java.util.List ;
import java.util.Map ;
import java.util.prefs.Preferences ;
import javax.imageio.ImageIO ;
import javax.swing.* ;
import javax.swing.border.EmptyBorder ;
import javax.swing.filechooser.FileNameExtensionFilter ;
import javax.swing.table.AbstractTableModel ;

public class GunPreviewer {
	static final int MAX_MUZZLE_FLASH_TYPES = 128 ;
	static final int MAX_BULLET_IMPACT_TYPES = 128 ;
	static final int MAX_PROJECTILE_STYLES = 16 ;
	static final int FIRST_CUSTOM_GUN_ID = 67 ;
	static final Color BACKGROUND = new Color(20, 23, 27) ;
	static final Color PANEL = new Color(29, 33, 38) ;
	static final Color PANEL_LIGHT = new Color(39, 44, 50) ;
	static final Color TEXT = new Color(233, 236, 239) ;
	static final Color MUTED = new Color(152, 161, 170) ;
	static final Color ACCENT = new Color(225, 83, 52) ;

	public static void main(String[] arguments) {
		SwingUtilities.invokeLater(new Runnable() {
			public void run() {
				try {UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName()) ;}
				catch (Exception ignored) {}
				new PreviewFrame().setVisible(true) ;
			}
		}) ;
	}

	static File resourceRoot() throws IOException {
		try {
			File location = new File(GunPreviewer.class.getProtectionDomain().getCodeSource().getLocation().toURI()) ;
			return location.isFile() ? location.getParentFile() : location ;
		} catch (URISyntaxException exception) {
			throw new IOException("Cannot locate the application folder", exception) ;
		}
	}

	static class WeaponInfo {
		final int id ;
		int damage, delay, clip, numClips, reloadDelay, cost, type, fireMode, pellets, scope, category, teams, flashStyle, impactStyle ;
		int projectileType, projectileStyle, explosionStyle ;
		int impactRed, impactGreen, impactBlue, impactFadeTime ;
		float spread, walkingSpeed, bulletSpeed, viewAngle, impactScale, explosionFrameTime, spriteOffsetX, spriteOffsetY ;
		String name ;

		WeaponInfo(String[] fields) {
			id = Integer.parseInt(fields[0]) ;
			damage = Integer.parseInt(fields[1]) ;
			delay = Integer.parseInt(fields[2]) ;
			spread = Float.parseFloat(fields[3]) ;
			clip = Integer.parseInt(fields[4]) ;
			numClips = Integer.parseInt(fields[5]) ;
			reloadDelay = Integer.parseInt(fields[6]) ;
			walkingSpeed = Float.parseFloat(fields[7]) ;
			bulletSpeed = Float.parseFloat(fields[8]) ;
			viewAngle = Float.parseFloat(fields[9]) ;
			cost = Integer.parseInt(fields[10]) ;
			type = Integer.parseInt(fields[11]) ;
			fireMode = Integer.parseInt(fields[12]) ;
			pellets = Integer.parseInt(fields[13]) ;
			scope = Integer.parseInt(fields[14]) ;
			category = Integer.parseInt(fields[15]) ;
			teams = Integer.parseInt(fields[16]) ;
			flashStyle = fields.length >= 19 ? Integer.parseInt(fields[17]) : 0 ;
			if (flashStyle < 0 || flashStyle >= MAX_MUZZLE_FLASH_TYPES) flashStyle = 0 ;
			impactStyle = fields.length >= 21 ? Integer.parseInt(fields[18]) : 0 ;
			if (impactStyle < 0 || impactStyle >= MAX_BULLET_IMPACT_TYPES) impactStyle = 0 ;
			impactScale = fields.length >= 21 ? Float.parseFloat(fields[19]) : 1.0f ;
			if (impactScale < 0.1f || impactScale > 10.0f) impactScale = 1.0f ;
			impactRed = fields.length >= 25 ? Integer.parseInt(fields[20]) : 255 ;
			impactGreen = fields.length >= 25 ? Integer.parseInt(fields[21]) : 128 ;
			impactBlue = fields.length >= 25 ? Integer.parseInt(fields[22]) : 35 ;
			impactFadeTime = fields.length >= 25 ? Integer.parseInt(fields[23]) : 250 ;
			if (impactRed < 0 || impactRed > 255) impactRed = 255 ;
			if (impactGreen < 0 || impactGreen > 255) impactGreen = 128 ;
			if (impactBlue < 0 || impactBlue > 255) impactBlue = 35 ;
			if (impactFadeTime < 1 || impactFadeTime > 60000) impactFadeTime = 250 ;
			projectileType = fields.length >= 26 ? Integer.parseInt(fields[24]) : 0 ;
			if (projectileType < 0 || projectileType > 1) projectileType = 0 ;
			projectileStyle = fields.length >= 27 ? Integer.parseInt(fields[25]) : 0 ;
			if (projectileStyle < 0 || projectileStyle >= MAX_PROJECTILE_STYLES) projectileStyle = 0 ;
			explosionStyle = fields.length >= 29 ? Integer.parseInt(fields[26]) : 0 ;
			if (explosionStyle < 0 || explosionStyle >= MAX_PROJECTILE_STYLES) explosionStyle = 0 ;
			explosionFrameTime = fields.length >= 29 ? Float.parseFloat(fields[27]) : 50.0f ;
			if (explosionFrameTime < 1.0f || explosionFrameTime > 60000.0f) explosionFrameTime = 50.0f ;
			spriteOffsetX = fields.length >= 31 ? Float.parseFloat(fields[28]) : 0.0f ;
			spriteOffsetY = fields.length >= 31 ? Float.parseFloat(fields[29]) : 0.0f ;
			if (spriteOffsetX < -100.0f || spriteOffsetX > 100.0f) spriteOffsetX = 0.0f ;
			if (spriteOffsetY < -100.0f || spriteOffsetY > 100.0f) spriteOffsetY = 0.0f ;
			name = fields[fields.length - 1] ;
		}

		String record() {
			return id + " " + damage + " " + delay + " " + decimal(spread) + " " + clip + " " + numClips
			       + " " + reloadDelay + " " + decimal(walkingSpeed) + " " + decimal(bulletSpeed) + " "
			       + decimal(viewAngle) + " " + cost + " " + type + " " + fireMode + " " + pellets + " "
			       + scope + " " + category + " " + teams + " " + flashStyle + " " + impactStyle + " "
			       + decimal(impactScale) + " " + impactRed + " " + impactGreen + " " + impactBlue + " "
			       + impactFadeTime + " " + projectileType + " " + projectileStyle + " " + explosionStyle + " "
			       + decimal(explosionFrameTime) + " " + decimal(spriteOffsetX) + " " + decimal(spriteOffsetY) + " " + name ;
		}

		static String decimal(float value) {return Float.toString(value) ;}

		static WeaponInfo createDefault(int id) {
			return new WeaponInfo((id + " 20 100 0.3 30 5 2000 1.0 1.0 0.0 1000 0 1 1 0 4 3 0 0 1.0 255 128 35 250 0 0 0 50.0 0.0 0.0 NEW-GUN").split(" ")) ;
		}

		String weaponType() {
			String[] names = {"PRIMARY", "SECONDARY", "KNIFE", "GRENADE"} ;
			return type >= 0 && type < names.length ? names[type] : "TYPE " + type ;
		}

		String bulletType() {
			if (projectileType == 1) return "ROCKET " + projectileStyle ;
			if (type == 2) return "MELEE" ;
			if (type == 3) return "GRENADE" ;
			if (pellets > 1) return "PELLETS x" + pellets ;
			return "BULLET" ;
		}

		public String toString() {return String.format("%02d  %s", id, name) ;}
	}

	private static class TracerConfig {
		String style = "line" ;
		int red = 255 ;
		int green = 165 ;
		int blue = 0 ;
		int alpha = 225 ;
		float length ;
		float width = 1.0f ;

		Color color() {return new Color(red, green, blue, alpha) ;}

		TracerConfig copy() {
			TracerConfig copy = new TracerConfig() ;
			copy.style = style ; copy.red = red ; copy.green = green ; copy.blue = blue ; copy.alpha = alpha ;
			copy.length = length ; copy.width = width ;
			return copy ;
		}

		String record(int id) {
			return id + " " + style + " " + red + " " + green + " " + blue + " " + alpha + " "
			       + Float.toString(length) + " " + Float.toString(width) ;
		}
	}

	private static class LaserConfig {
		boolean enabled ;
		boolean falloffEnabled ;
		int red = 255 ;
		int green = 32 ;
		int blue = 24 ;
		int alpha = 180 ;
		float width = 1.0f ;
		float range = 700.0f ;
		float falloff = 1.0f ;
		boolean endDot = true ;
		float endDotScale = 1.0f ;
		float offsetX = 12.0f ;
		float offsetY ;

		Color color() {return new Color(red, green, blue, alpha) ;}

		LaserConfig copy() {
			LaserConfig copy = new LaserConfig() ;
			copy.enabled = enabled ; copy.falloffEnabled = falloffEnabled ;
			copy.red = red ; copy.green = green ; copy.blue = blue ; copy.alpha = alpha ;
			copy.width = width ; copy.range = range ; copy.falloff = falloff ;
			copy.endDot = endDot ; copy.endDotScale = endDotScale ; copy.offsetX = offsetX ; copy.offsetY = offsetY ;
			return copy ;
		}

		String record(int id) {
			return id + " " + (enabled ? 1 : 0) + " " + (falloffEnabled ? 1 : 0) + " "
			       + red + " " + green + " " + blue + " " + alpha + " " + Float.toString(width) + " "
			       + Float.toString(range) + " " + Float.toString(falloff) + " " + (endDot ? 1 : 0) + " "
			       + Float.toString(endDotScale) + " " + Float.toString(offsetX) + " " + Float.toString(offsetY) ;
		}
	}

	private static class ScreenShakeConfig {
		boolean fireEnabled, explosionEnabled ;
		int fireMagnitude = 4, fireTime = 120, explosionMagnitude = 12, explosionTime = 500 ;
		float explosionRadius = 500.0f ;

		ScreenShakeConfig copy() {
			ScreenShakeConfig copy = new ScreenShakeConfig() ;
			copy.fireEnabled = fireEnabled ; copy.explosionEnabled = explosionEnabled ;
			copy.fireMagnitude = fireMagnitude ; copy.fireTime = fireTime ;
			copy.explosionMagnitude = explosionMagnitude ; copy.explosionTime = explosionTime ;
			copy.explosionRadius = explosionRadius ;
			return copy ;
		}

		String record(int id) {
			return id + " " + (fireEnabled ? fireMagnitude : 0) + " " + fireTime + " "
			       + (explosionEnabled ? explosionMagnitude : 0) + " " + explosionTime + " " + Float.toString(explosionRadius) ;
		}
	}

	private static class TracerShot {
		final int weaponId ;
		final long startedAt ;
		final double angle ;

		TracerShot(int weaponId, long startedAt, double angle) {
			this.weaponId = weaponId ; this.startedAt = startedAt ; this.angle = angle ;
		}
	}

	private static class AnimationFrame {
		int duration ;
		final double[] angles ;

		AnimationFrame(int duration, double[] angles) {
			this.duration = duration ;
			this.angles = angles ;
		}

		AnimationFrame copy() {return new AnimationFrame(duration,angles.clone()) ;}
	}

	static class ResourceModel {
		static final Preferences PREFERENCES = Preferences.userNodeForPackage(GunPreviewer.class) ;
		final List<WeaponInfo> weapons = new ArrayList<WeaponInfo>() ;
		final BufferedImage[] handPages = new BufferedImage[2] ;
		final BufferedImage[] groundPages = new BufferedImage[2] ;
		final BufferedImage[] impactPages = new BufferedImage[2] ;
		BufferedImage projectileAtlas ;
		final BufferedImage[] explosionAtlases = new BufferedImage[MAX_PROJECTILE_STYLES] ;
		final Map<Integer, BufferedImage> flashes = new HashMap<Integer, BufferedImage>() ;
		final Map<Integer, TracerConfig> tracers = new HashMap<Integer, TracerConfig>() ;
		final Map<Integer, LaserConfig> lasers = new HashMap<Integer, LaserConfig>() ;
		final Map<Integer, ScreenShakeConfig> screenShakes = new HashMap<Integer, ScreenShakeConfig>() ;
		final Map<Integer, String> weaponAnimationProfiles = new HashMap<Integer, String>() ;
		final Map<Integer, Float> projectileExplosionRadii = new HashMap<Integer, Float>() ;
		final Map<Integer, Integer> projectileLaunchOrigins = new HashMap<Integer, Integer>() ;
		final Map<Integer, float[]> muzzlePositions = new HashMap<Integer, float[]>() ;
		final Map<String, List<AnimationFrame>> animations = new LinkedHashMap<String, List<AnimationFrame>>() ;
		BufferedImage players ;
		File directory ;
		File graphicsDirectory ;
		File dataDirectory ;
		File requestedRoot ;
		final List<String> trailingLines = new ArrayList<String>() ;
		final List<String> tracerLines = new ArrayList<String>() ;
		final List<String> laserLines = new ArrayList<String>() ;
		final List<String> screenShakeLines = new ArrayList<String>() ;
		final List<String> weaponAnimationLines = new ArrayList<String>() ;
		final List<String> projectileExplosionLines = new ArrayList<String>() ;
		final List<String> projectileLaunchOriginLines = new ArrayList<String>() ;
		final List<String> muzzlePositionLines = new ArrayList<String>() ;

		void load() throws IOException {
			if (requestedRoot == null) {
				String saved = PREFERENCES.get("gameRoot", "") ;
				requestedRoot = saved.length() == 0 ? resourceRoot() : migrateSavedRoot(new File(saved)) ;
			}
			directory = resolveContentRoot(requestedRoot) ;
			graphicsDirectory = new File(directory, "gfx") ;
			dataDirectory = new File(directory, "data") ;
			weapons.clear() ;
			flashes.clear() ;
			tracers.clear() ;
			lasers.clear() ;
			screenShakes.clear() ;
			weaponAnimationProfiles.clear() ;
			projectileExplosionRadii.clear() ;
			projectileLaunchOrigins.clear() ;
			muzzlePositions.clear() ;
			animations.clear() ;
			trailingLines.clear() ;
			tracerLines.clear() ;
			laserLines.clear() ;
			screenShakeLines.clear() ;
			weaponAnimationLines.clear() ;
			projectileExplosionLines.clear() ;
			projectileLaunchOriginLines.clear() ;
			muzzlePositionLines.clear() ;
			handPages[0] = readRequired("guns.png") ;
			handPages[1] = readOptional("guns2.png") ;
			groundPages[0] = readRequired("gunsground.png") ;
			groundPages[1] = readOptional("gunsground2.png") ;
			impactPages[0] = readRequired("bulletimpacts.png") ;
			impactPages[1] = readOptional("bulletimpacts2.png") ;
			projectileAtlas = readRequired("rocketprojectile.png") ;
			explosionAtlases[0] = readRequired("explosionsprites.png") ;
			for (int style=1; style<MAX_PROJECTILE_STYLES; style++) explosionAtlases[style] = readOptional("explosionsprites" + style + ".png") ;
			players = readRequired("players.png") ;
			loadMuzzleFlashes() ;
			validateAtlas(handPages[0], "guns.png") ;
			validateAtlas(groundPages[0], "gunsground.png") ;
			if (handPages[1] != null) validateAtlas(handPages[1], "guns2.png") ;
			if (groundPages[1] != null) validateAtlas(groundPages[1], "gunsground2.png") ;
			validateAtlas(impactPages[0], "bulletimpacts.png") ;
			if (impactPages[1] != null) validateAtlas(impactPages[1], "bulletimpacts2.png") ;
			validateFourByFourAtlas(projectileAtlas,"rocketprojectile.png") ;
			for (int style=0; style<MAX_PROJECTILE_STYLES; style++) {
				if (explosionAtlases[style] != null) validateFourByFourAtlas(explosionAtlases[style],style == 0 ? "explosionsprites.png" : "explosionsprites" + style + ".png") ;
			}
			loadWeapons() ;
			loadAnimations() ;
			loadTracers() ;
			loadLasers() ;
			loadScreenShakes() ;
			loadWeaponAnimationProfiles() ;
			loadProjectileExplosionRadii() ;
			loadProjectileLaunchOrigins() ;
			loadMuzzlePositions() ;
		}

		File resolveContentRoot(File root) throws IOException {
			File applicationRoot = resourceRoot() ;
			if (root.equals(applicationRoot) && hasContentFolders(root)) return root ;
			File bin = new File(root, "bin") ;
			if (hasContentFolders(bin)) return bin ;
			throw new IOException("Select the game folder immediately above bin. Expected bin/gfx and bin/data inside: " + root.getAbsolutePath()) ;
		}

		File migrateSavedRoot(File root) {
			if ("bin".equalsIgnoreCase(root.getName()) && hasContentFolders(root) && root.getParentFile() != null) {
				root = root.getParentFile() ;
				PREFERENCES.put("gameRoot", root.getAbsolutePath()) ;
			}
			return root ;
		}

		boolean hasContentFolders(File root) {
			return root != null && new File(root, "gfx").isDirectory() && new File(root, "data").isDirectory() ;
		}

		void setRoot(File root) {
			requestedRoot = root ;
			PREFERENCES.put("gameRoot", root.getAbsolutePath()) ;
		}

		BufferedImage readRequired(String name) throws IOException {
			BufferedImage image = readOptional(name) ;
			if (image == null) throw new IOException("Missing or invalid resource: " + new File(graphicsDirectory, name).getAbsolutePath()) ;
			return image ;
		}

		BufferedImage readOptional(String name) throws IOException {
			File file = new File(graphicsDirectory, name) ;
			if (!file.isFile()) return null ;
			BufferedImage image = ImageIO.read(file) ;
			if (image == null) throw new IOException("Unsupported image: " + file.getAbsolutePath()) ;
			return image ;
		}

		void loadMuzzleFlashes() throws IOException {
			Map<Integer, String> paths = new HashMap<Integer, String>() ;
			paths.put(Integer.valueOf(0), "gfx/muzzleflash.png") ;
			File resources = new File(dataDirectory, "resources.txt") ;
			if (resources.isFile()) {
				BufferedReader reader = new BufferedReader(new FileReader(resources)) ;
				try {
					String line ;
					while ((line = reader.readLine()) != null) {
						int comment = line.indexOf('#') ;
						if (comment >= 0) line = line.substring(0, comment) ;
						int separator = line.indexOf('=') ;
						if (separator < 0) continue ;
						String key = line.substring(0, separator).trim() ;
						int style ;
						if (key.equals("muzzle_flash")) style = 0 ;
						else if (key.startsWith("muzzle_flash_")) {
							try {style = Integer.parseInt(key.substring(13)) ;}
							catch (NumberFormatException ignored) {continue ;}
						} else continue ;
						if (style >= 0 && style < MAX_MUZZLE_FLASH_TYPES) paths.put(Integer.valueOf(style), line.substring(separator + 1).trim()) ;
					}
				} finally {reader.close() ;}
			}
			for (Map.Entry<Integer, String> entry : paths.entrySet()) {
				File file = new File(directory, entry.getValue().replace('/', File.separatorChar)) ;
				if (!file.isFile()) {
					if (entry.getKey().intValue() == 0) throw new IOException("Missing muzzle flash: " + file.getAbsolutePath()) ;
					continue ;
				}
				BufferedImage image = ImageIO.read(file) ;
				if (image == null) throw new IOException("Unsupported image: " + file.getAbsolutePath()) ;
				flashes.put(entry.getKey(), image) ;
			}
		}

		void validateAtlas(BufferedImage image, String name) throws IOException {
			if (image.getWidth() % 32 != 0 || image.getHeight() % 32 != 0) throw new IOException(name + " must contain 32x32 cells") ;
		}

		void validateFourByFourAtlas(BufferedImage image, String name) throws IOException {
			if (image.getWidth() != 128 || image.getHeight() != 128) throw new IOException(name + " must be a 4x4 atlas of 32x32 cells") ;
		}

		BufferedImage explosionAtlas(int style) {
			if (style >= 0 && style < explosionAtlases.length && explosionAtlases[style] != null) return explosionAtlases[style] ;
			return null ;
		}

		String explosionAtlasName(int style) {
			return style == 0 ? "explosionsprites.png" : "explosionsprites" + style + ".png" ;
		}

		void loadWeapons() throws IOException {
			File file = new File(dataDirectory, "guns.txt") ;
			if (!file.isFile()) throw new IOException("Missing guns data: " + file.getAbsolutePath()) ;
			BufferedReader reader = new BufferedReader(new FileReader(file)) ;
			try {
				String first = reader.readLine() ;
				if (first == null) throw new IOException("guns.txt is empty") ;
				int count = Integer.parseInt(first.trim()) ;
				String line ;
				while (weapons.size() < count && (line = reader.readLine()) != null) {
					line = line.trim() ;
					if (line.length() == 0 || line.startsWith("#")) continue ;
					String[] fields = line.split("\\s+") ;
					if (fields.length < 18) throw new IOException("Invalid guns.txt record: " + line) ;
					weapons.add(new WeaponInfo(fields)) ;
				}
				if (weapons.size() != count) throw new IOException("guns.txt declares " + count + " weapons but contains " + weapons.size()) ;
				while ((line = reader.readLine()) != null) trailingLines.add(line) ;
			} catch (NumberFormatException exception) {
				throw new IOException("Invalid number in guns.txt", exception) ;
			} finally {reader.close() ;}
		}

		void loadAnimations() throws IOException {
			File file = new File(dataDirectory,"animations.txt") ;
			if (!file.isFile()) throw new IOException("Missing animation data: " + file.getAbsolutePath()) ;
			BufferedReader reader = new BufferedReader(new FileReader(file)) ;
			String currentName = null ;
			int lineNumber = 0 ;
			try {
				String line ;
				while ((line = reader.readLine()) != null) {
					lineNumber++ ;
					int comment = line.indexOf('#') ;
					if (comment >= 0) line = line.substring(0,comment) ;
					line = line.trim() ;
					if (line.length() == 0) continue ;
					if (line.endsWith("{")) {
						currentName = line.substring(0,line.length()-1).trim().toUpperCase() ;
						if (currentName.length() == 0) throw new IOException("Missing animation name at animations.txt line " + lineNumber) ;
						animations.put(currentName,new ArrayList<AnimationFrame>()) ;
						continue ;
					}
					if (line.equals("}")) {currentName = null ; continue ;}
					if (currentName == null) throw new IOException("Animation keyframe outside a block at animations.txt line " + lineNumber) ;
					String[] fields = line.split("\\s+") ;
					if (fields.length != 7) throw new IOException("Expected duration and six angles at animations.txt line " + lineNumber) ;
					int duration = Integer.parseInt(fields[0]) ;
					double[] angles = new double[6] ;
					for (int index=0; index<angles.length; index++) angles[index] = Double.parseDouble(fields[index+1]) ;
					animations.get(currentName).add(new AnimationFrame(duration,angles)) ;
				}
			} catch (NumberFormatException exception) {
				throw new IOException("Invalid number at animations.txt line " + lineNumber,exception) ;
			} finally {reader.close() ;}
			String[] required = {"PRIMARY","PRIMARY_FIRE","PRIMARY_RELOAD","SECONDARY","SECONDARY_FIRE","SECONDARY_RELOAD","KNIFE","KNIFE_SLASH","GRENADE","GRENADE_PULLBACK"} ;
			for (String name : required) {
				List<AnimationFrame> frames = animations.get(name) ;
				if (frames == null || frames.isEmpty()) throw new IOException("animations.txt is missing the " + name + " pose") ;
			}
		}

		AnimationFrame animationFrame(String name, int index) {
			List<AnimationFrame> frames = animations.get(name) ;
			if (frames == null || frames.isEmpty()) frames = animations.get("PRIMARY") ;
			if (frames == null || frames.isEmpty()) return new AnimationFrame(1,new double[6]) ;
			return frames.get(Math.max(0,Math.min(frames.size()-1,index))) ;
		}

		String idleAnimationName(WeaponInfo weapon) {
			String assigned = weaponAnimationProfile(weapon.id) ;
			List<AnimationFrame> assignedFrames = assigned == null ? null : animations.get(assigned) ;
			if (assignedFrames != null && !assignedFrames.isEmpty()) return assigned ;
			String[] names = {"PRIMARY","SECONDARY","KNIFE","GRENADE"} ;
			return names[Math.max(0,Math.min(names.length-1,weapon.type))] ;
		}

		List<String> animationProfiles() {
			List<String> profiles = new ArrayList<String>() ;
			for (String name : animations.keySet()) {
				if (!name.endsWith("_FIRE") && !name.endsWith("_RELOAD") && animations.containsKey(name+"_FIRE") && animations.containsKey(name+"_RELOAD")) profiles.add(name) ;
			}
			return profiles ;
		}

		void addAnimationProfile(String name) throws IOException {duplicateAnimationProfile("PRIMARY",name) ;}

		void duplicateAnimationProfile(String sourceName, String name) throws IOException {
			String profile = name.trim().toUpperCase() ;
			if (!profile.matches("[A-Z][A-Z0-9_]{0,31}") || profile.endsWith("_FIRE") || profile.endsWith("_RELOAD")) {
				throw new IOException("Profile names must use 1-32 letters, numbers, or underscores and cannot end in _FIRE or _RELOAD.") ;
			}
			if (animations.containsKey(profile)) throw new IOException("Animation profile already exists: " + profile) ;
			String[] targets = {profile,profile+"_FIRE",profile+"_RELOAD"} ;
			String[] sources = {sourceName,sourceName+"_FIRE",sourceName+"_RELOAD"} ;
			for (int action=0; action<targets.length; action++) {
				if (!animations.containsKey(sources[action])) throw new IOException("Animation block is missing: " + sources[action]) ;
				List<AnimationFrame> frames = new ArrayList<AnimationFrame>() ;
				for (AnimationFrame frame : animations.get(sources[action])) frames.add(frame.copy()) ;
				animations.put(targets[action],frames) ;
			}
		}

		void deleteAnimationProfile(String name) throws IOException {
			if ("PRIMARY".equals(name) || "SECONDARY".equals(name)) throw new IOException("Built-in animation profiles cannot be removed.") ;
			animations.remove(name) ;
			animations.remove(name+"_FIRE") ;
			animations.remove(name+"_RELOAD") ;
			List<Integer> clearedIds = new ArrayList<Integer>() ;
			for (Map.Entry<Integer,String> entry : weaponAnimationProfiles.entrySet()) {
				if (name.equals(entry.getValue())) clearedIds.add(entry.getKey()) ;
			}
			for (Integer id : clearedIds) {
				weaponAnimationProfiles.remove(id) ;
				removeConfigRecord(weaponAnimationLines,id.intValue(),2) ;
			}
			saveAnimations() ;
			writeLinesWithBackup(new File(dataDirectory,"weaponanimations.txt"),weaponAnimationLines) ;
		}

		void saveAnimations() throws IOException {
			File target = new File(dataDirectory,"animations.txt") ;
			File temporary = new File(dataDirectory,"animations.txt.tmp") ;
			PrintWriter writer = new PrintWriter(new OutputStreamWriter(new FileOutputStream(temporary),"UTF-8")) ;
			try {
				writer.println("# WEAPON POSE KEYFRAMES") ;
				writer.println("# Format: duration_ms body right_arm right_hand left_arm left_hand gun") ;
				writer.println("# Profile triplets use NAME, NAME_FIRE, and NAME_RELOAD blocks.") ;
				for (Map.Entry<String,List<AnimationFrame>> entry : animations.entrySet()) {
					writer.println(entry.getKey() + " {") ;
					for (AnimationFrame frame : entry.getValue()) {
						writer.print("\t" + frame.duration) ;
						for (double angle : frame.angles) writer.print(" " + formatAnimationNumber(angle)) ;
						writer.println() ;
					}
					writer.println("}") ;
				}
			} finally {writer.close() ;}
			File backup = new File(dataDirectory,"animations.txt.bak") ;
			if (backup.exists() && !backup.delete()) throw new IOException("Cannot replace " + backup.getAbsolutePath()) ;
			if (!target.renameTo(backup)) throw new IOException("Cannot back up " + target.getAbsolutePath()) ;
			if (!temporary.renameTo(target)) {backup.renameTo(target) ; throw new IOException("Cannot save " + target.getAbsolutePath()) ;}
		}

		String formatAnimationNumber(double value) {
			if (value == Math.rint(value)) return Integer.toString((int)value) ;
			return Double.toString(value) ;
		}

		void saveWeapons() throws IOException {
			File target = new File(dataDirectory, "guns.txt") ;
			File temporary = new File(dataDirectory, "guns.txt.tmp") ;
			PrintWriter writer = new PrintWriter(new OutputStreamWriter(new FileOutputStream(temporary), "UTF-8")) ;
			try {
				writer.println(weapons.size()) ;
				writer.println("# WEAPONS: the first line is the record count and must remain first.") ;
				writer.println("# Format: id damage delay spread clip magazines reload_ms speed bullet_speed view_angle cost inventory_type fire_mode pellets scope buy_category buy_teams muzzle_flash impact_style impact_scale impact_r impact_g impact_b impact_fade_ms projectile_type projectile_style explosion_style explosion_frame_ms sprite_x sprite_y name") ;
				writer.println("# inventory_type: 0=primary 1=secondary 2=knife 3=grenade. fire_mode: 0=semi 1=automatic.") ;
				writer.println("# scope: 0=none 1=low 2=medium 3=high. buy_category: 0=hidden 1=pistols 2=shotguns 3=SMG 4=rifles 5=machine_guns 6=equipment 7=special.") ;
				writer.println("# buy_teams is a bitmask: 0=none 1=T 2=CT 3=both. projectile_type: 0=bullet 1=rocket.") ;
				for (WeaponInfo weapon : weapons) writer.println(weapon.record()) ;
				for (String line : trailingLines) writer.println(line) ;
			} finally {writer.close() ;}
			File backup = new File(dataDirectory, "guns.txt.bak") ;
			if (backup.exists() && !backup.delete()) throw new IOException("Cannot replace " + backup.getAbsolutePath()) ;
			if (!target.renameTo(backup)) throw new IOException("Cannot back up " + target.getAbsolutePath()) ;
			if (!temporary.renameTo(target)) {
				backup.renameTo(target) ;
				throw new IOException("Cannot save " + target.getAbsolutePath()) ;
			}
		}

		void removeLastWeapon() throws IOException {
			if (weapons.isEmpty()) throw new IOException("There are no weapons to remove.") ;
			WeaponInfo weapon = weapons.get(weapons.size()-1) ;
			if (weapon.id < FIRST_CUSTOM_GUN_ID) throw new IOException("Built-in weapons cannot be removed.") ;
			weapons.remove(weapons.size()-1) ;
			removeConfigRecord(tracerLines,weapon.id,8) ;
			removeConfigRecord(laserLines,weapon.id,12,14) ;
			removeConfigRecord(screenShakeLines,weapon.id,6) ;
			removeConfigRecord(weaponAnimationLines,weapon.id,2) ;
			removeConfigRecord(projectileExplosionLines,weapon.id,2) ;
			removeConfigRecord(projectileLaunchOriginLines,weapon.id,2) ;
			removeConfigRecord(muzzlePositionLines,weapon.id,3) ;
			try {
				saveWeapons() ;
				writeLinesWithBackup(new File(dataDirectory,"tracers.txt"),tracerLines) ;
				if (!laserLines.isEmpty() || new File(dataDirectory,"lasers.txt").isFile()) {
					writeLinesWithBackup(new File(dataDirectory,"lasers.txt"),laserLines) ;
				}
				if (!screenShakeLines.isEmpty() || new File(dataDirectory,"screenshakes.txt").isFile()) {
					writeLinesWithBackup(new File(dataDirectory,"screenshakes.txt"),screenShakeLines) ;
				}
				if (!weaponAnimationLines.isEmpty() || new File(dataDirectory,"weaponanimations.txt").isFile()) {
					writeLinesWithBackup(new File(dataDirectory,"weaponanimations.txt"),weaponAnimationLines) ;
				}
				if (!projectileExplosionLines.isEmpty() || new File(dataDirectory,"projectileexplosions.txt").isFile()) {
					writeLinesWithBackup(new File(dataDirectory,"projectileexplosions.txt"),projectileExplosionLines) ;
				}
				if (!projectileLaunchOriginLines.isEmpty() || new File(dataDirectory,"projectileorigins.txt").isFile()) {
					writeLinesWithBackup(new File(dataDirectory,"projectileorigins.txt"),projectileLaunchOriginLines) ;
				}
				if (!muzzlePositionLines.isEmpty() || new File(dataDirectory,"muzzlepositions.txt").isFile()) {
					writeLinesWithBackup(new File(dataDirectory,"muzzlepositions.txt"),muzzlePositionLines) ;
				}
				tracers.remove(Integer.valueOf(weapon.id)) ;
				lasers.remove(Integer.valueOf(weapon.id)) ;
				screenShakes.remove(Integer.valueOf(weapon.id)) ;
				weaponAnimationProfiles.remove(Integer.valueOf(weapon.id)) ;
				projectileExplosionRadii.remove(Integer.valueOf(weapon.id)) ;
				projectileLaunchOrigins.remove(Integer.valueOf(weapon.id)) ;
				muzzlePositions.remove(Integer.valueOf(weapon.id)) ;
			} catch (IOException exception) {
				try {load() ;}
				catch (IOException ignored) {}
				throw exception ;
			}
		}

		void removeConfigRecord(List<String> lines, int id, int firstFieldCount, int secondFieldCount) {
			for (int index=lines.size()-1; index>=0; index--) {
				String trimmed = lines.get(index).trim() ;
				if (trimmed.length() == 0 || trimmed.startsWith("#")) continue ;
				String[] fields = trimmed.split("\\s+") ;
				if (fields.length != firstFieldCount && fields.length != secondFieldCount) continue ;
				try {if (Integer.parseInt(fields[0]) == id) lines.remove(index) ;}
				catch (NumberFormatException ignored) {}
			}
		}

		void removeConfigRecord(List<String> lines, int id, int fieldCount) {
			removeConfigRecord(lines,id,fieldCount,fieldCount) ;
		}

		void loadTracers() throws IOException {
			File file = new File(dataDirectory, "tracers.txt") ;
			if (!file.isFile()) throw new IOException("Missing tracer data: " + file.getAbsolutePath()) ;
			BufferedReader reader = new BufferedReader(new FileReader(file)) ;
			try {
				String line ;
				while ((line = reader.readLine()) != null) {
					tracerLines.add(line) ;
					String trimmed = line.trim() ;
					if (trimmed.length() == 0 || trimmed.startsWith("#")) continue ;
					String[] fields = trimmed.split("\\s+") ;
					if (fields.length != 8) continue ;
					TracerConfig tracer = new TracerConfig() ;
					int id = Integer.parseInt(fields[0]) ;
					tracer.style = fields[1].toLowerCase() ;
					tracer.red = Integer.parseInt(fields[2]) ; tracer.green = Integer.parseInt(fields[3]) ;
					tracer.blue = Integer.parseInt(fields[4]) ; tracer.alpha = Integer.parseInt(fields[5]) ;
					tracer.length = Float.parseFloat(fields[6]) ; tracer.width = Float.parseFloat(fields[7]) ;
					if (id >= 0 && id < 128) tracers.put(Integer.valueOf(id), tracer) ;
				}
			} catch (NumberFormatException exception) {
				throw new IOException("Invalid number in tracers.txt", exception) ;
			} finally {reader.close() ;}
		}

		TracerConfig tracer(int id) {
			TracerConfig tracer = tracers.get(Integer.valueOf(id)) ;
			return tracer == null ? new TracerConfig() : tracer.copy() ;
		}

		void saveTracer(int id, TracerConfig tracer) throws IOException {
			String record = tracer.record(id) ;
			boolean replaced = false ;
			int insertAt = 0 ;
			for (int index = 0 ; index < tracerLines.size() ; index++) {
				String trimmed = tracerLines.get(index).trim() ;
				if (trimmed.length() == 0 || trimmed.startsWith("#")) continue ;
				String[] fields = trimmed.split("\\s+") ;
				try {
					if (fields.length == 8) {
						insertAt = index + 1 ;
						if (Integer.parseInt(fields[0]) == id && !replaced) {
							tracerLines.set(index, record) ;
							replaced = true ;
						}
					}
				} catch (NumberFormatException ignored) {}
			}
			if (!replaced) tracerLines.add(insertAt, record) ;
			writeLinesWithBackup(new File(dataDirectory, "tracers.txt"), tracerLines) ;
			tracers.put(Integer.valueOf(id), tracer.copy()) ;
		}

		void loadLasers() throws IOException {
			File file = new File(dataDirectory, "lasers.txt") ;
			if (!file.isFile()) return ;
			BufferedReader reader = new BufferedReader(new FileReader(file)) ;
			try {
				String line ;
				while ((line = reader.readLine()) != null) {
					laserLines.add(line) ;
					String trimmed = line.trim() ;
					if (trimmed.length() == 0 || trimmed.startsWith("#")) continue ;
					String[] fields = trimmed.split("\\s+") ;
					if (fields.length != 12 && fields.length != 14) continue ;
					LaserConfig laser = new LaserConfig() ;
					int id = Integer.parseInt(fields[0]) ;
					laser.enabled = Integer.parseInt(fields[1]) != 0 ;
					laser.falloffEnabled = Integer.parseInt(fields[2]) != 0 ;
					laser.red = clamp(Integer.parseInt(fields[3]), 0, 255) ; laser.green = clamp(Integer.parseInt(fields[4]), 0, 255) ;
					laser.blue = clamp(Integer.parseInt(fields[5]), 0, 255) ; laser.alpha = clamp(Integer.parseInt(fields[6]), 0, 255) ;
					laser.width = clamp(Float.parseFloat(fields[7]), 0.1f, 20.0f) ; laser.range = clamp(Float.parseFloat(fields[8]), 1.0f, 5000.0f) ;
					laser.falloff = clamp(Float.parseFloat(fields[9]), 0.0f, 10.0f) ; laser.endDot = Integer.parseInt(fields[10]) != 0 ;
					laser.endDotScale = clamp(Float.parseFloat(fields[11]), 0.1f, 20.0f) ;
					if (fields.length == 14) {
						laser.offsetX = clamp(Float.parseFloat(fields[12]), -100.0f, 100.0f) ;
						laser.offsetY = clamp(Float.parseFloat(fields[13]), -100.0f, 100.0f) ;
					}
					if (id >= 0 && id < 128) lasers.put(Integer.valueOf(id), laser) ;
				}
			} catch (NumberFormatException exception) {
				throw new IOException("Invalid number in lasers.txt", exception) ;
			} finally {reader.close() ;}
		}

		LaserConfig laser(int id) {
			LaserConfig laser = lasers.get(Integer.valueOf(id)) ;
			return laser == null ? new LaserConfig() : laser.copy() ;
		}

		int clamp(int value, int minimum, int maximum) {return Math.max(minimum, Math.min(maximum, value)) ;}
		float clamp(float value, float minimum, float maximum) {return Math.max(minimum, Math.min(maximum, value)) ;}

		void saveLaser(int id, LaserConfig laser) throws IOException {
			String record = laser.record(id) ;
			boolean replaced = false ;
			int insertAt = 0 ;
			for (int index = 0 ; index < laserLines.size() ; index++) {
				String trimmed = laserLines.get(index).trim() ;
				if (trimmed.length() == 0 || trimmed.startsWith("#")) continue ;
				String[] fields = trimmed.split("\\s+") ;
				try {
					if (fields.length == 12 || fields.length == 14) {
						insertAt = index + 1 ;
						if (Integer.parseInt(fields[0]) == id && !replaced) {
							laserLines.set(index, record) ;
							replaced = true ;
						}
					}
				} catch (NumberFormatException ignored) {}
			}
			if (!replaced) laserLines.add(insertAt, record) ;
			if (laserLines.isEmpty()) {
				laserLines.add("# id enabled falloff_enabled red green blue alpha width range falloff end_dot end_dot_scale offset_x offset_y") ;
				laserLines.add(record) ;
			}
			writeLinesWithBackup(new File(dataDirectory, "lasers.txt"), laserLines) ;
			lasers.put(Integer.valueOf(id), laser.copy()) ;
		}

		void loadScreenShakes() throws IOException {
			File file = new File(dataDirectory, "screenshakes.txt") ;
			if (!file.isFile()) return ;
			BufferedReader reader = new BufferedReader(new FileReader(file)) ;
			try {
				String line ;
				while ((line = reader.readLine()) != null) {
					screenShakeLines.add(line) ;
					String trimmed = line.trim() ;
					if (trimmed.length() == 0 || trimmed.startsWith("#")) continue ;
					String[] fields = trimmed.split("\\s+") ;
					if (fields.length != 6) continue ;
					int id = Integer.parseInt(fields[0]) ;
					ScreenShakeConfig config = new ScreenShakeConfig() ;
					config.fireMagnitude = clamp(Integer.parseInt(fields[1]),0,100) ;
					config.fireEnabled = config.fireMagnitude > 0 ;
					config.fireTime = clamp(Integer.parseInt(fields[2]),0,5000) ;
					config.explosionMagnitude = clamp(Integer.parseInt(fields[3]),0,100) ;
					config.explosionEnabled = config.explosionMagnitude > 0 ;
					config.explosionTime = clamp(Integer.parseInt(fields[4]),0,5000) ;
					config.explosionRadius = clamp(Float.parseFloat(fields[5]),0.0f,5000.0f) ;
					if (id >= 0 && id < 128) screenShakes.put(Integer.valueOf(id),config) ;
				}
			} catch (NumberFormatException exception) {
				throw new IOException("Invalid number in screenshakes.txt",exception) ;
			} finally {reader.close() ;}
		}

		ScreenShakeConfig screenShake(int id) {
			ScreenShakeConfig config = screenShakes.get(Integer.valueOf(id)) ;
			return config == null ? new ScreenShakeConfig() : config.copy() ;
		}

		void saveScreenShake(int id, ScreenShakeConfig config) throws IOException {
			String record = config.record(id) ;
			boolean replaced = false ;
			for (int index=0; index<screenShakeLines.size(); index++) {
				String[] fields = screenShakeLines.get(index).trim().split("\\s+") ;
				try {
					if (fields.length == 6 && Integer.parseInt(fields[0]) == id) {
						screenShakeLines.set(index,record) ; replaced = true ; break ;
					}
				} catch (NumberFormatException ignored) {}
			}
			if (screenShakeLines.isEmpty()) screenShakeLines.add("# id fire_magnitude fire_time_ms explosion_magnitude explosion_time_ms explosion_radius") ;
			if (!replaced) screenShakeLines.add(record) ;
			writeLinesWithBackup(new File(dataDirectory,"screenshakes.txt"),screenShakeLines) ;
			screenShakes.put(Integer.valueOf(id),config.copy()) ;
		}

		void loadWeaponAnimationProfiles() throws IOException {
			File file = new File(dataDirectory,"weaponanimations.txt") ;
			if (!file.isFile()) return ;
			BufferedReader reader = new BufferedReader(new FileReader(file)) ;
			try {
				String line ;
				while ((line = reader.readLine()) != null) {
					weaponAnimationLines.add(line) ;
					String trimmed = line.trim() ;
					if (trimmed.length() == 0 || trimmed.startsWith("#")) continue ;
					String[] fields = trimmed.split("\\s+") ;
					if (fields.length != 2) continue ;
					int id = Integer.parseInt(fields[0]) ;
					String profile = fields[1].toUpperCase() ;
					if ("0".equals(profile)) continue ;
					if ("1".equals(profile)) profile = "SPECIAL" ;
					if (id >= 0 && id < 128) weaponAnimationProfiles.put(Integer.valueOf(id),profile) ;
				}
			} catch (NumberFormatException exception) {
				throw new IOException("Invalid weapon ID in weaponanimations.txt",exception) ;
			} finally {reader.close() ;}
		}

		String weaponAnimationProfile(int id) {
			return weaponAnimationProfiles.get(Integer.valueOf(id)) ;
		}

		void saveWeaponAnimationProfile(int id, String profile) throws IOException {
			removeConfigRecord(weaponAnimationLines,id,2) ;
			if (weaponAnimationLines.isEmpty()) weaponAnimationLines.add("# id profile_name. Missing IDs use their inventory animation.") ;
			if (profile != null) weaponAnimationLines.add(id + " " + profile) ;
			writeLinesWithBackup(new File(dataDirectory,"weaponanimations.txt"),weaponAnimationLines) ;
			if (profile == null) weaponAnimationProfiles.remove(Integer.valueOf(id)) ;
			else weaponAnimationProfiles.put(Integer.valueOf(id),profile) ;
		}

		void loadProjectileExplosionRadii() throws IOException {
			File file = new File(dataDirectory,"projectileexplosions.txt") ;
			if (!file.isFile()) return ;
			BufferedReader reader = new BufferedReader(new FileReader(file)) ;
			try {
				String line ;
				while ((line = reader.readLine()) != null) {
					projectileExplosionLines.add(line) ;
					String trimmed = line.trim() ;
					if (trimmed.length() == 0 || trimmed.startsWith("#")) continue ;
					String[] fields = trimmed.split("\\s+") ;
					if (fields.length != 2) continue ;
					int id = Integer.parseInt(fields[0]) ;
					float radius = clamp(Float.parseFloat(fields[1]),0.0f,5000.0f) ;
					if (id >= 0 && id < 128) projectileExplosionRadii.put(Integer.valueOf(id),Float.valueOf(radius)) ;
				}
			} catch (NumberFormatException exception) {
				throw new IOException("Invalid number in projectileexplosions.txt",exception) ;
			} finally {reader.close() ;}
		}

		Float projectileExplosionRadius(int id) {
			return projectileExplosionRadii.get(Integer.valueOf(id)) ;
		}

		void saveProjectileExplosionRadius(int id, Float radius) throws IOException {
			removeConfigRecord(projectileExplosionLines,id,2) ;
			if (projectileExplosionLines.isEmpty()) {
				projectileExplosionLines.add("# PROJECTILE SPLASH RADII") ;
				projectileExplosionLines.add("# Format: id radius. Missing IDs use the global HE radius; zero disables splash.") ;
			}
			if (radius != null) projectileExplosionLines.add(id + " " + WeaponInfo.decimal(radius.floatValue())) ;
			writeLinesWithBackup(new File(dataDirectory,"projectileexplosions.txt"),projectileExplosionLines) ;
			if (radius == null) projectileExplosionRadii.remove(Integer.valueOf(id)) ;
			else projectileExplosionRadii.put(Integer.valueOf(id),radius) ;
		}

		void loadProjectileLaunchOrigins() throws IOException {
			File file = new File(dataDirectory,"projectileorigins.txt") ;
			if (!file.isFile()) return ;
			BufferedReader reader = new BufferedReader(new FileReader(file)) ;
			try {
				String line ;
				while ((line = reader.readLine()) != null) {
					projectileLaunchOriginLines.add(line) ;
					String trimmed = line.trim() ;
					if (trimmed.length() == 0 || trimmed.startsWith("#")) continue ;
					String[] fields = trimmed.split("\\s+") ;
					if (fields.length != 2) continue ;
					int id = Integer.parseInt(fields[0]) ;
					int origin = Integer.parseInt(fields[1]) == 1 ? 1 : 0 ;
					if (id >= 0 && id < 128) projectileLaunchOrigins.put(Integer.valueOf(id),Integer.valueOf(origin)) ;
				}
			} catch (NumberFormatException exception) {
				throw new IOException("Invalid number in projectileorigins.txt",exception) ;
			} finally {reader.close() ;}
		}

		int projectileLaunchOrigin(WeaponInfo weapon) {
			Integer origin = projectileLaunchOrigins.get(Integer.valueOf(weapon.id)) ;
			return origin == null ? (weapon.projectileType == 1 ? 1 : 0) : origin.intValue() ;
		}

		void saveProjectileLaunchOrigin(int id, int origin) throws IOException {
			removeConfigRecord(projectileLaunchOriginLines,id,2) ;
			if (projectileLaunchOriginLines.isEmpty()) {
				projectileLaunchOriginLines.add("# PROJECTILE LAUNCH ORIGINS") ;
				projectileLaunchOriginLines.add("# Format: id origin. 0 = standard, 1 = shoulder.") ;
			}
			projectileLaunchOriginLines.add(id + " " + (origin == 1 ? 1 : 0)) ;
			writeLinesWithBackup(new File(dataDirectory,"projectileorigins.txt"),projectileLaunchOriginLines) ;
			projectileLaunchOrigins.put(Integer.valueOf(id),Integer.valueOf(origin == 1 ? 1 : 0)) ;
		}

		void loadMuzzlePositions() throws IOException {
			File file = new File(dataDirectory,"muzzlepositions.txt") ;
			if (!file.isFile()) return ;
			BufferedReader reader = new BufferedReader(new FileReader(file)) ;
			try {
				String line ;
				while ((line = reader.readLine()) != null) {
					muzzlePositionLines.add(line) ;
					String trimmed = line.trim() ;
					if (trimmed.length() == 0 || trimmed.startsWith("#")) continue ;
					String[] fields = trimmed.split("\\s+") ;
					if (fields.length != 3) continue ;
					int id = Integer.parseInt(fields[0]) ;
					float forward = clamp(Float.parseFloat(fields[1]),-100.0f,100.0f) ;
					float sideways = clamp(Float.parseFloat(fields[2]),-100.0f,100.0f) ;
					if (id >= 0 && id < 128) muzzlePositions.put(Integer.valueOf(id),new float[] {forward,sideways}) ;
				}
			} catch (NumberFormatException exception) {
				throw new IOException("Invalid number in muzzlepositions.txt",exception) ;
			} finally {reader.close() ;}
		}

		float[] muzzlePosition(int id) {
			float[] position = muzzlePositions.get(Integer.valueOf(id)) ;
			return position == null ? new float[] {0.0f,0.0f} : new float[] {position[0],position[1]} ;
		}

		void saveMuzzlePosition(int id, float forward, float sideways) throws IOException {
			removeConfigRecord(muzzlePositionLines,id,3) ;
			if (muzzlePositionLines.isEmpty()) {
				muzzlePositionLines.add("# MUZZLE FLASH POSITIONS") ;
				muzzlePositionLines.add("# Format: id forward sideways. Offsets are relative to the animated gun anchor.") ;
			}
			muzzlePositionLines.add(id + " " + WeaponInfo.decimal(forward) + " " + WeaponInfo.decimal(sideways)) ;
			writeLinesWithBackup(new File(dataDirectory,"muzzlepositions.txt"),muzzlePositionLines) ;
			muzzlePositions.put(Integer.valueOf(id),new float[] {forward,sideways}) ;
		}

		void writeLinesWithBackup(File target, List<String> lines) throws IOException {
			File temporary = new File(target.getParentFile(), target.getName() + ".tmp") ;
			PrintWriter writer = new PrintWriter(new OutputStreamWriter(new FileOutputStream(temporary), "UTF-8")) ;
			try {for (String line : lines) writer.println(line) ;}
			finally {writer.close() ;}
			File backup = new File(target.getParentFile(), target.getName() + ".bak") ;
			boolean hadTarget = target.isFile() ;
			if (hadTarget && backup.exists() && !backup.delete()) throw new IOException("Cannot replace " + backup.getAbsolutePath()) ;
			if (hadTarget && !target.renameTo(backup)) throw new IOException("Cannot back up " + target.getAbsolutePath()) ;
			if (!temporary.renameTo(target)) {
				if (hadTarget) backup.renameTo(target) ;
				throw new IOException("Cannot save " + target.getAbsolutePath()) ;
			}
		}

		void replaceWeaponSprite(int id, File source, boolean hand) throws IOException {
			BufferedImage replacement = ImageIO.read(source) ;
			if (replacement == null) throw new IOException("The selected file is not a supported image.") ;
			if (replacement.getWidth() != 32 || replacement.getHeight() != 32) {
				throw new IOException("Weapon sprites must be exactly 32x32 pixels.") ;
			}
			BufferedImage[] pages = hand ? handPages : groundPages ;
			BufferedImage atlas = page(pages, id) ;
			int pageIndex = id / 64 ;
			if (atlas == null && pageIndex >= 0 && pageIndex < pages.length) {
				atlas = new BufferedImage(128, 512, BufferedImage.TYPE_INT_ARGB) ;
				pages[pageIndex] = atlas ;
			}
			Rectangle targetCell = cell(atlas, id) ;
			if (atlas == null || targetCell == null) throw new IOException("No atlas cell exists for weapon ID " + id + ".") ;
			String atlasName = atlasName(hand, id / 64) ;
			File target = new File(graphicsDirectory, atlasName) ;
			File temporary = new File(graphicsDirectory, atlasName + ".tmp") ;
			File backup = new File(graphicsDirectory, atlasName + ".bak") ;
			BufferedImage updated = new BufferedImage(atlas.getWidth(), atlas.getHeight(), BufferedImage.TYPE_INT_ARGB) ;
			Graphics2D graphics = updated.createGraphics() ;
			try {
				graphics.setComposite(AlphaComposite.Src) ;
				graphics.drawImage(atlas, 0, 0, null) ;
				graphics.drawImage(replacement, targetCell.x, targetCell.y, null) ;
			} finally {graphics.dispose() ;}
			if (!ImageIO.write(updated, "png", temporary)) throw new IOException("PNG writing is unavailable.") ;
			boolean hadTarget = target.isFile() ;
			if (hadTarget && backup.exists() && !backup.delete()) throw new IOException("Cannot replace " + backup.getAbsolutePath()) ;
			if (hadTarget && !target.renameTo(backup)) throw new IOException("Cannot back up " + target.getAbsolutePath()) ;
			if (!temporary.renameTo(target)) {
				if (hadTarget) backup.renameTo(target) ;
				throw new IOException("Cannot save " + target.getAbsolutePath()) ;
			}
			pages[id / 64] = updated ;
		}

		void replaceImpactSprite(int style, File source) throws IOException {
			if (!source.getName().toLowerCase().endsWith(".png")) throw new IOException("Bullet-impact sprites must be PNG files.") ;
			BufferedImage replacement = ImageIO.read(source) ;
			if (replacement == null) throw new IOException("The selected file is not a supported image.") ;
			if (replacement.getWidth() != 32 || replacement.getHeight() != 32) {
				throw new IOException("Bullet-impact sprites must be exactly 32x32 pixels.") ;
			}
			int pageIndex = style / 64 ;
			if (pageIndex < 0 || pageIndex >= impactPages.length) throw new IOException("Impact style must be between 0 and 127.") ;
			BufferedImage atlas = impactPages[pageIndex] ;
			if (atlas == null) atlas = new BufferedImage(128,512,BufferedImage.TYPE_INT_ARGB) ;
			Rectangle targetCell = cell(atlas,style) ;
			if (targetCell == null) throw new IOException("No atlas cell exists for impact style " + style + ".") ;
			String atlasName = "bulletimpacts" + (pageIndex == 0 ? "" : Integer.toString(pageIndex+1)) + ".png" ;
			File target = new File(graphicsDirectory,atlasName) ;
			File temporary = new File(graphicsDirectory,atlasName + ".tmp") ;
			File backup = new File(graphicsDirectory,atlasName + ".bak") ;
			BufferedImage updated = new BufferedImage(atlas.getWidth(),atlas.getHeight(),BufferedImage.TYPE_INT_ARGB) ;
			Graphics2D graphics = updated.createGraphics() ;
			try {
				graphics.setComposite(AlphaComposite.Src) ;
				graphics.drawImage(atlas,0,0,null) ;
				graphics.drawImage(replacement,targetCell.x,targetCell.y,null) ;
			} finally {graphics.dispose() ;}
			if (!ImageIO.write(updated,"png",temporary)) throw new IOException("PNG writing is unavailable.") ;
			boolean hadTarget = target.isFile() ;
			if (hadTarget && backup.exists() && !backup.delete()) throw new IOException("Cannot replace " + backup.getAbsolutePath()) ;
			if (hadTarget && !target.renameTo(backup)) throw new IOException("Cannot back up " + target.getAbsolutePath()) ;
			if (!temporary.renameTo(target)) {
				if (hadTarget) backup.renameTo(target) ;
				throw new IOException("Cannot save " + target.getAbsolutePath()) ;
			}
			impactPages[pageIndex] = updated ;
		}

		String atlasName(boolean hand, int page) {
			String base = hand ? "guns" : "gunsground" ;
			return base + (page == 0 ? "" : Integer.toString(page + 1)) + ".png" ;
		}

		BufferedImage page(BufferedImage[] pages, int id) {
			int page = id / 64 ;
			return page >= 0 && page < pages.length ? pages[page] : null ;
		}

		BufferedImage handPage(int id) {return page(handPages, id) ;}
		BufferedImage groundPage(int id) {return page(groundPages, id) ;}
		BufferedImage impactPage(int style) {
			BufferedImage selected = page(impactPages, style) ;
			return selected == null || cell(selected, style) == null ? impactPages[0] : selected ;
		}

		Rectangle impactCell(int style) {
			BufferedImage selected = impactPage(style) ;
			Rectangle selectedCell = cell(selected, style) ;
			return selectedCell == null ? cell(impactPages[0], 0) : selectedCell ;
		}

		Rectangle cell(BufferedImage page, int id) {
			if (page == null) return null ;
			int localId = id % 64 ;
			int columns = page.getWidth() / 32 ;
			int x = (localId % columns) * 32 ;
			int y = (localId / columns) * 32 ;
			return x + 32 <= page.getWidth() && y + 32 <= page.getHeight() ? new Rectangle(x, y, 32, 32) : null ;
		}

		BufferedImage flash(int style) {
			BufferedImage image = flashes.get(Integer.valueOf(style)) ;
			return image == null ? flashes.get(Integer.valueOf(0)) : image ;
		}
	}

	static class WeaponEditorDialog extends JDialog {
		static final String[] TRACER_STYLES = {"none", "line", "laser", "dot", "dashed", "beam", "plasma", "bolt",
		                                                "comet", "pulse", "rail", "spark", "needle", "twin", "zigzag",
		                                                "flare", "streak", "slug", "blade", "gauss", "lightning",
		                                                "spiral", "wave", "chain", "ricochet", "penetrator", "charge_beam",
		                                                "fading_beam", "expanding_beam", "tapered_beam", "gradient_beam", "heat_ray", "disruptor",
		                                                "particle_trail", "smoke_trail", "impact_ring", "impact_burst", "afterimage", "animated_texture"} ;
		final PreviewFrame frame ;
		final WeaponInfo weapon ;
		final boolean newWeapon ;
		final TracerConfig tracer ;
		final LaserConfig laser ;
		final ScreenShakeConfig screenShake ;
		File pendingHandSprite, pendingGroundSprite ;
		final JTextField name = new JTextField() ;
		final JSpinner damage = integer(0, 10000), delay = integer(0, 60000), clip = integer(0, 10000) ;
		final JSpinner numClips = integer(0, 1000), reloadDelay = integer(0, 60000), cost = integer(0, 100000) ;
		final JSpinner pellets = integer(1, 1000), flashStyle = integer(0, MAX_MUZZLE_FLASH_TYPES - 1) ;
		final JSpinner impactStyle = integer(0, MAX_BULLET_IMPACT_TYPES - 1), impactScale = decimal(0.1, 10.0, 0.1) ;
		final JSpinner impactRed = integer(0, 255), impactGreen = integer(0, 255), impactBlue = integer(0, 255) ;
		final JSpinner impactFadeTime = integer(1, 60000) ;
		final JSpinner projectileStyle = integer(0, MAX_PROJECTILE_STYLES - 1) ;
		final JSpinner explosionStyle = integer(0, MAX_PROJECTILE_STYLES - 1), explosionFrameTime = integer(1, 60000) ;
		final JSpinner projectileExplosionRadius = decimal(0.0,5000.0,10.0) ;
		final JLabel explosionDuration = new JLabel() ;
		final JSpinner tracerRed = integer(0, 255), tracerGreen = integer(0, 255), tracerBlue = integer(0, 255), tracerAlpha = integer(0, 255) ;
		final JSpinner spread = decimal(0.0, 100.0, 0.01), walkingSpeed = decimal(0.0, 100.0, 0.05) ;
		final JSpinner bulletSpeed = decimal(0.0, 100.0, 0.05), viewAngle = decimal(0.0, 10.0, 0.01) ;
		final JSpinner spriteOffsetX = decimal(-100.0, 100.0, 0.5), spriteOffsetY = decimal(-100.0, 100.0, 0.5) ;
		final JSpinner muzzleForward = decimal(-100.0,100.0,0.5), muzzleSideways = decimal(-100.0,100.0,0.5) ;
		final JSpinner fireShakeMagnitude = integer(1,100), fireShakeTime = integer(1,5000) ;
		final JSpinner explosionShakeMagnitude = integer(1,100), explosionShakeTime = integer(1,5000) ;
		final JSpinner explosionShakeRadius = decimal(1.0,5000.0,10.0) ;
		final JSpinner tracerLength = decimal(0.0, 5000.0, 1.0), tracerWidth = decimal(0.1, 20.0, 0.1) ;
		final JSpinner laserRed = integer(0, 255), laserGreen = integer(0, 255), laserBlue = integer(0, 255), laserAlpha = integer(0, 255) ;
		final JSpinner laserWidth = decimal(0.1, 20.0, 0.1), laserRange = decimal(1.0, 5000.0, 10.0) ;
		final JSpinner laserFalloff = decimal(0.0, 10.0, 0.1), laserEndDotScale = decimal(0.1, 20.0, 0.1) ;
		final JSpinner laserOffsetX = decimal(-100.0, 100.0, 0.5), laserOffsetY = decimal(-100.0, 100.0, 0.5) ;
		final JCheckBox laserEnabled = new JCheckBox("Enabled"), laserFalloffEnabled = new JCheckBox("Use intensity falloff") ;
		final JCheckBox laserEndDot = new JCheckBox("Display endpoint dot") ;
		final JCheckBox tracerLightBackground = new JCheckBox("Light background") ;
		final JCheckBox impactLightBackground = new JCheckBox("Light background") ;
		final JCheckBox fireShakeEnabled = new JCheckBox("Enabled on successful shot") ;
		final JCheckBox explosionShakeEnabled = new JCheckBox("Enabled on explosion impact") ;
		final JCheckBox overrideProjectileExplosionRadius = new JCheckBox("Override global HE radius") ;
		final JComboBox<String> type = new JComboBox<String>(new String[] {"Primary", "Secondary", "Knife", "Grenade"}) ;
		final JComboBox<String> animationType = new JComboBox<String>() ;
		final JComboBox<String> projectileType = new JComboBox<String>(new String[] {"Bullet", "Rocket"}) ;
		final JComboBox<String> projectileLaunchOrigin = new JComboBox<String>(new String[] {"Standard", "Shoulder"}) ;
		final JComboBox<String> fireMode = new JComboBox<String>(new String[] {"Semi-auto", "Automatic"}) ;
		final JComboBox<String> scope = new JComboBox<String>(new String[] {"None", "Low", "Medium", "High"}) ;
		final JComboBox<String> category = new JComboBox<String>(new String[] {"Hidden", "Pistols", "Shotguns", "SMG", "Rifles", "Machine guns", "Equipment", "Special"}) ;
		final JComboBox<String> teams = new JComboBox<String>(new String[] {"None", "Terrorist", "Counter-Terrorist", "Both"}) ;
		final JComboBox<String> tracerStyle = new JComboBox<String>(TRACER_STYLES) ;
		final JSpinner tracerPreviewZoom = new JSpinner(new SpinnerNumberModel(2, 1, 8, 1)) ;
		final TracerPreviewPanel tracerPreview = new TracerPreviewPanel() ;
		final ColorSwatch tracerColorSwatch = new ColorSwatch() ;
		final JSpinner laserPreviewZoom = new JSpinner(new SpinnerNumberModel(2, 1, 8, 1)) ;
		final LaserPreviewPanel laserPreview = new LaserPreviewPanel() ;
		final LaserColorSwatch laserColorSwatch = new LaserColorSwatch() ;
		final ImpactPreviewPanel impactPreview = new ImpactPreviewPanel() ;
		final ImpactColorSwatch impactColorSwatch = new ImpactColorSwatch() ;
		final ProjectilePreviewPanel projectilePreview = new ProjectilePreviewPanel() ;
		final WeaponPosePreviewPanel weaponPosePreview = new WeaponPosePreviewPanel() ;
		final AnimationEditorPanel animationEditor ;

		WeaponEditorDialog(PreviewFrame frame, WeaponInfo weapon) {this(frame, weapon, false) ;}

		WeaponEditorDialog(PreviewFrame frame, WeaponInfo weapon, boolean newWeapon) {
			super(frame, (newWeapon ? "Add weapon " : "Configure ") + weapon.name, true) ;
			this.frame = frame ;
			this.weapon = weapon ;
			this.newWeapon = newWeapon ;
			this.tracer = frame.model.tracer(weapon.id) ;
			this.laser = frame.model.laser(weapon.id) ;
			this.screenShake = frame.model.screenShake(weapon.id) ;
			this.animationEditor = new AnimationEditorPanel() ;
			setDefaultCloseOperation(DISPOSE_ON_CLOSE) ;
			JPanel content = new JPanel(new BorderLayout(12, 12)) ;
			content.setBorder(new EmptyBorder(16, 16, 16, 16)) ;
			JPanel weaponFields = formPanel() ;
			add(weaponFields, "ID (sprite cell)", new JLabel(Integer.toString(weapon.id))) ;
			add(weaponFields, "Name", name) ;
			add(weaponFields, "Damage", damage) ;
			add(weaponFields, "Shot delay (ms)", delay) ;
			add(weaponFields, "Spread (radians)", spread) ;
			add(weaponFields, "Magazine rounds", clip) ;
			add(weaponFields, "Magazine count", numClips) ;
			add(weaponFields, "Reload delay (ms)", reloadDelay) ;
			add(weaponFields, "Walking speed", walkingSpeed) ;
			add(weaponFields, "Bullet speed", bulletSpeed) ;
			add(weaponFields, "View angle", viewAngle) ;
			add(weaponFields, "Cost", cost) ;
			add(weaponFields, "Inventory type", type) ;
			add(weaponFields, "Fire mode", fireMode) ;
			add(weaponFields, "Pellets per shot", pellets) ;
			add(weaponFields, "Scope", scope) ;
			add(weaponFields, "Buy category", category) ;
			add(weaponFields, "Available teams", teams) ;
			add(weaponFields, "Muzzle flash ID", flashStyle) ;
			add(weaponFields, "Projectile type", projectileType) ;
			add(weaponFields, "Launch origin", projectileLaunchOrigin) ;
			add(weaponFields, "Projectile style (0-15)", projectileStyle) ;
			JPanel sprites = new JPanel(new FlowLayout(FlowLayout.LEFT)) ;
			sprites.setBorder(BorderFactory.createTitledBorder(newWeapon ? "Required 32x32 sprites" : "32x32 sprite replacement")) ;
			JButton importHand = new JButton("Import hand PNG") ;
			importHand.setToolTipText("Replace this weapon's 32x32 cell in the hand atlas") ;
			importHand.addActionListener(event -> importSprite(true)) ;
			JButton importGround = new JButton("Import ground PNG") ;
			importGround.setToolTipText("Replace this weapon's 32x32 cell in the ground atlas") ;
			importGround.addActionListener(event -> importSprite(false)) ;
			sprites.add(new JLabel("Weapon ID " + weapon.id)) ;
			sprites.add(importHand) ;
			sprites.add(importGround) ;
			JPanel weaponPage = new JPanel(new BorderLayout(8, 8)) ;
			weaponPage.setBorder(new EmptyBorder(10, 10, 10, 10)) ;
			weaponPage.add(scrollable(weaponFields), BorderLayout.CENTER) ;
			weaponPage.add(sprites, BorderLayout.SOUTH) ;

			JPanel poseFields = formPanel() ;
			add(poseFields, "Animation type", animationType) ;
			add(poseFields, "Sprite X (forward)", spriteOffsetX) ;
			add(poseFields, "Sprite Y (sideways)", spriteOffsetY) ;
			add(poseFields, "Muzzle forward", muzzleForward) ;
			add(poseFields, "Muzzle sideways", muzzleSideways) ;
			JPanel posePage = new JPanel(new BorderLayout(0, 10)) ;
			posePage.setBorder(new EmptyBorder(10, 10, 10, 10)) ;
			posePage.add(weaponPosePreview, BorderLayout.CENTER) ;
			posePage.add(poseFields, BorderLayout.SOUTH) ;

			JPanel tracerFields = formPanel() ;
			add(tracerFields, "Tracer type", tracerStyle) ;
			add(tracerFields, "Red (0-255)", tracerRed) ;
			add(tracerFields, "Green (0-255)", tracerGreen) ;
			add(tracerFields, "Blue (0-255)", tracerBlue) ;
			add(tracerFields, "Alpha (0-255)", tracerAlpha) ;
			add(tracerFields, "Trail length", tracerLength) ;
			add(tracerFields, "Line width", tracerWidth) ;
			JButton chooseTracerColor = new JButton("Choose color...") ;
			chooseTracerColor.addActionListener(event -> chooseTracerColor()) ;
			JPanel colorPicker = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 0)) ;
			colorPicker.add(tracerColorSwatch) ;
			colorPicker.add(chooseTracerColor) ;
			colorPicker.add(tracerLightBackground) ;
			JPanel tracerPreviewArea = new JPanel(new BorderLayout(12, 0)) ;
			tracerPreviewArea.setBorder(BorderFactory.createTitledBorder("Live tracer preview")) ;
			tracerPreviewArea.add(tracerPreview, BorderLayout.CENTER) ;
			tracerPreviewArea.add(colorPicker, BorderLayout.EAST) ;
			tracerPreviewArea.add(previewZoomBar(tracerPreviewZoom,tracerPreview), BorderLayout.SOUTH) ;
			JPanel tracerPage = new JPanel(new BorderLayout(0, 10)) ;
			tracerPage.setBorder(new EmptyBorder(10, 10, 10, 10)) ;
			tracerPage.add(tracerPreviewArea, BorderLayout.NORTH) ;
			tracerPage.add(scrollable(tracerFields), BorderLayout.CENTER) ;

			JPanel impactFields = formPanel() ;
			add(impactFields, "Impact style ID", impactStyle) ;
			add(impactFields, "Impact scale", impactScale) ;
			add(impactFields, "Red (0-255)", impactRed) ;
			add(impactFields, "Green (0-255)", impactGreen) ;
			add(impactFields, "Blue (0-255)", impactBlue) ;
			add(impactFields, "Fade time (ms)", impactFadeTime) ;
			JButton chooseImpactColor = new JButton("Choose color...") ;
			chooseImpactColor.addActionListener(event -> chooseImpactColor()) ;
			JButton importImpact = new JButton("Import impact PNG") ;
			importImpact.setToolTipText("Replace the selected impact style with an exact 32x32 PNG; grayscale art works best with tinting") ;
			importImpact.addActionListener(event -> importImpactSprite()) ;
			add(impactFields,"Impact artwork",importImpact) ;
			JPanel impactColorPicker = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 0)) ;
			impactColorPicker.add(impactColorSwatch) ;
			impactColorPicker.add(chooseImpactColor) ;
			impactColorPicker.add(impactLightBackground) ;
			JPanel impactPreviewArea = new JPanel(new BorderLayout(12, 0)) ;
			impactPreviewArea.setBorder(BorderFactory.createTitledBorder("Live bullet-impact preview")) ;
			impactPreviewArea.add(impactPreview, BorderLayout.CENTER) ;
			impactPreviewArea.add(impactColorPicker, BorderLayout.EAST) ;
			JPanel impactPage = new JPanel(new BorderLayout(0, 10)) ;
			impactPage.setBorder(new EmptyBorder(10, 10, 10, 10)) ;
			impactPage.add(impactPreviewArea, BorderLayout.NORTH) ;
			impactPage.add(scrollable(impactFields), BorderLayout.CENTER) ;

			JPanel projectileFields = formPanel() ;
			add(projectileFields,"Explosion style (0-15)",explosionStyle) ;
			add(projectileFields,"Explosion frame time (ms)",explosionFrameTime) ;
			add(projectileFields,"Total animation duration",explosionDuration) ;
			add(projectileFields,"Damage radius override",overrideProjectileExplosionRadius) ;
			add(projectileFields,"Explosion damage radius",projectileExplosionRadius) ;
			JPanel projectilePage = new JPanel(new BorderLayout(0,10)) ;
			projectilePage.setBorder(new EmptyBorder(10,10,10,10)) ;
			projectilePage.add(projectilePreview,BorderLayout.NORTH) ;
			projectilePage.add(projectileFields,BorderLayout.CENTER) ;

			JPanel laserFields = formPanel() ;
			add(laserFields, "Laser", laserEnabled) ;
			add(laserFields, "Use falloff", laserFalloffEnabled) ;
			add(laserFields, "Falloff exponent", laserFalloff) ;
			add(laserFields, "Red (0-255)", laserRed) ;
			add(laserFields, "Green (0-255)", laserGreen) ;
			add(laserFields, "Blue (0-255)", laserBlue) ;
			add(laserFields, "Alpha (0-255)", laserAlpha) ;
			add(laserFields, "Line width", laserWidth) ;
			add(laserFields, "Maximum range", laserRange) ;
			add(laserFields, "Endpoint dot", laserEndDot) ;
			add(laserFields, "Endpoint scale", laserEndDotScale) ;
			add(laserFields, "Emitter X offset", laserOffsetX) ;
			add(laserFields, "Emitter Y offset", laserOffsetY) ;
			JButton chooseLaserColor = new JButton("Choose color...") ;
			chooseLaserColor.addActionListener(event -> chooseLaserColor()) ;
			JPanel laserColorPicker = new JPanel(new FlowLayout(FlowLayout.LEFT, 10, 0)) ;
			laserColorPicker.add(laserColorSwatch) ;
			laserColorPicker.add(chooseLaserColor) ;
			JPanel laserPreviewArea = new JPanel(new BorderLayout(12, 0)) ;
			laserPreviewArea.setBorder(BorderFactory.createTitledBorder("Live laser preview")) ;
			laserPreviewArea.add(laserPreview, BorderLayout.CENTER) ;
			laserPreviewArea.add(laserColorPicker, BorderLayout.EAST) ;
			laserPreviewArea.add(previewZoomBar(laserPreviewZoom,laserPreview), BorderLayout.SOUTH) ;
			JPanel laserPage = new JPanel(new BorderLayout(0, 10)) ;
			laserPage.setBorder(new EmptyBorder(10, 10, 10, 10)) ;
			laserPage.add(laserPreviewArea, BorderLayout.NORTH) ;
			laserPage.add(scrollable(laserFields), BorderLayout.CENTER) ;

			JPanel shakeFields = formPanel() ;
			add(shakeFields,"Launch shake",fireShakeEnabled) ;
			add(shakeFields,"Launch magnitude",fireShakeMagnitude) ;
			add(shakeFields,"Launch duration (ms)",fireShakeTime) ;
			add(shakeFields,"Explosion shake",explosionShakeEnabled) ;
			add(shakeFields,"Explosion magnitude",explosionShakeMagnitude) ;
			add(shakeFields,"Explosion duration (ms)",explosionShakeTime) ;
			add(shakeFields,"Explosion radius",explosionShakeRadius) ;
			JPanel shakePage = new JPanel(new BorderLayout()) ;
			shakePage.setBorder(new EmptyBorder(10,10,10,10)) ;
			shakePage.add(shakeFields,BorderLayout.NORTH) ;

			JTabbedPane pages = new JTabbedPane(JTabbedPane.LEFT,JTabbedPane.SCROLL_TAB_LAYOUT) ;
			pages.addTab("Weapon", weaponPage) ;
			pages.addTab("Pose & Muzzle", posePage) ;
			pages.addTab("Animations", animationEditor) ;
			pages.addTab("Tracer", tracerPage) ;
			pages.addTab("Laser", laserPage) ;
			pages.addTab("Impact", impactPage) ;
			pages.addTab("Projectile", projectilePage) ;
			pages.addTab("Screen Shake", shakePage) ;
			content.add(pages, BorderLayout.CENTER) ;
			JPanel actions = new JPanel(new FlowLayout(FlowLayout.RIGHT)) ;
			JButton cancel = new JButton("Cancel") ;
			cancel.addActionListener(event -> dispose()) ;
			JButton save = new JButton(newWeapon ? "Add gun" : "Save weapon") ;
			save.addActionListener(event -> save()) ;
			actions.add(cancel) ;
			actions.add(save) ;
			content.add(actions, BorderLayout.SOUTH) ;
			setContentPane(content) ;
			loadValues() ;
			installTracerPreviewListeners() ;
			installLaserPreviewListeners() ;
			installImpactPreviewListeners() ;
			projectileType.addActionListener(event -> {refreshProjectileControls() ; projectilePreview.repaint() ;}) ;
			overrideProjectileExplosionRadius.addActionListener(event -> refreshProjectileControls()) ;
			type.addActionListener(event -> weaponPosePreview.repaint()) ;
			animationType.addActionListener(event -> weaponPosePreview.repaint()) ;
			spriteOffsetX.addChangeListener(event -> weaponPosePreview.repaint()) ;
			spriteOffsetY.addChangeListener(event -> weaponPosePreview.repaint()) ;
			muzzleForward.addChangeListener(event -> weaponPosePreview.repaint()) ;
			muzzleSideways.addChangeListener(event -> weaponPosePreview.repaint()) ;
			projectileStyle.addChangeListener(event -> projectilePreview.repaint()) ;
			explosionStyle.addChangeListener(event -> {refreshProjectileControls() ; projectilePreview.repaint() ;}) ;
			explosionFrameTime.addChangeListener(event -> {refreshProjectileControls() ; projectilePreview.repaint() ;}) ;
			final Timer impactTimer = new Timer(16, event -> {impactPreview.repaint() ; projectilePreview.repaint() ;}) ;
			impactTimer.start() ;
			addWindowListener(new WindowAdapter() {public void windowClosed(WindowEvent event) {impactTimer.stop() ; animationEditor.stop() ;}}) ;
			setSize(920, 680) ;
			setMinimumSize(new Dimension(780, 560)) ;
			setLocationRelativeTo(frame) ;
		}

		static JSpinner integer(int minimum, int maximum) {return new JSpinner(new SpinnerNumberModel(minimum, minimum, maximum, 1)) ;}
		static JSpinner decimal(double minimum, double maximum, double step) {return new JSpinner(new SpinnerNumberModel(minimum, minimum, maximum, step)) ;}

		class AnimationEditorPanel extends JPanel {
			final JComboBox<String> profile = new JComboBox<String>() ;
			final JComboBox<String> action = new JComboBox<String>(new String[] {"Idle","Fire","Reload"}) ;
			final AnimationTableModel tableModel = new AnimationTableModel() ;
			final JTable table = new JTable(tableModel) ;
			final AnimationPreviewPanel preview = new AnimationPreviewPanel() ;
			final JButton play = new JButton("Pause") ;
			final Timer previewTimer ;
			AnimationFrame copiedFrame ;
			boolean playing = true ;
			long lastTick = System.currentTimeMillis() ;
			int animationFrame ;
			double frameTime ;

			AnimationEditorPanel() {
				super(new BorderLayout(8,8)) ;
				setBorder(new EmptyBorder(10,10,10,10)) ;
				JPanel toolbar = new JPanel(new GridLayout(2,1,0,5)) ;
				JPanel profileTools = new JPanel(new FlowLayout(FlowLayout.LEFT,8,0)) ;
				JPanel keyframeTools = new JPanel(new FlowLayout(FlowLayout.LEFT,8,0)) ;
				profileTools.add(new JLabel("Profile")) ; profileTools.add(profile) ;
				profileTools.add(new JLabel("Action")) ; profileTools.add(action) ;
				JButton addProfile = new JButton("Add profile") ;
				addProfile.addActionListener(event -> addProfile()) ;
				JButton duplicateProfile = new JButton("Duplicate profile") ;
				duplicateProfile.addActionListener(event -> duplicateProfile()) ;
				JButton deleteProfile = new JButton("Delete profile") ;
				deleteProfile.addActionListener(event -> deleteProfile()) ;
				JButton addFrame = new JButton("Add keyframe") ;
				addFrame.addActionListener(event -> tableModel.addFrame()) ;
				JButton removeFrame = new JButton("Remove keyframe") ;
				removeFrame.addActionListener(event -> tableModel.removeFrame(table.getSelectedRow())) ;
				JButton copyFrame = new JButton("Copy keyframe") ;
				copyFrame.addActionListener(event -> tableModel.copyFrame(table.getSelectedRow())) ;
				JButton pasteFrame = new JButton("Paste keyframe") ;
				pasteFrame.addActionListener(event -> tableModel.pasteFrame(table.getSelectedRow())) ;
				JButton saveAnimations = new JButton("Save animations") ;
				saveAnimations.addActionListener(event -> saveAnimations()) ;
				play.addActionListener(event -> {
					playing = !playing ; play.setText(playing ? "Pause" : "Play") ;
					lastTick = System.currentTimeMillis() ; preview.repaint() ;
				}) ;
				profileTools.add(addProfile) ; profileTools.add(duplicateProfile) ; profileTools.add(deleteProfile) ;
				keyframeTools.add(play) ; keyframeTools.add(addFrame) ; keyframeTools.add(removeFrame) ;
				keyframeTools.add(copyFrame) ; keyframeTools.add(pasteFrame) ; keyframeTools.add(saveAnimations) ;
				toolbar.add(profileTools) ; toolbar.add(keyframeTools) ;
				add(toolbar,BorderLayout.NORTH) ;
				table.setAutoResizeMode(JTable.AUTO_RESIZE_ALL_COLUMNS) ;
				table.setRowHeight(24) ;
				JScrollPane tableScroll = new JScrollPane(table) ;
				JSplitPane split = new JSplitPane(JSplitPane.VERTICAL_SPLIT,preview,tableScroll) ;
				split.setResizeWeight(0.55) ; split.setDividerLocation(250) ;
				add(split,BorderLayout.CENTER) ;
				for (String name : frame.model.animationProfiles()) profile.addItem(name) ;
				profile.addActionListener(event -> {tableModel.selectBlock() ; restartPreview() ;}) ;
				action.addActionListener(event -> {tableModel.selectBlock() ; restartPreview() ;}) ;
				table.getSelectionModel().addListSelectionListener(event -> {
					if (!event.getValueIsAdjusting() && table.getSelectedRow() >= 0) {
						playing = false ; play.setText("Play") ; animationFrame = table.getSelectedRow() ; frameTime = 0.0 ; preview.repaint() ;
					}
				}) ;
				tableModel.selectBlock() ;
				previewTimer = new Timer(16,event -> updatePreview()) ;
				previewTimer.start() ;
			}

			void stop() {previewTimer.stop() ;}

			void restartPreview() {
				animationFrame = 0 ; frameTime = 0.0 ; lastTick = System.currentTimeMillis() ;
				table.clearSelection() ; preview.repaint() ;
			}

			void updatePreview() {
				long now = System.currentTimeMillis() ;
				double elapsed = Math.min(100.0,now-lastTick) ; lastTick = now ;
				if (playing && !tableModel.frames.isEmpty()) {
					frameTime += elapsed ;
					while (frameTime >= tableModel.frames.get(animationFrame).duration) {
						frameTime -= tableModel.frames.get(animationFrame).duration ;
						animationFrame = (animationFrame+1)%tableModel.frames.size() ;
					}
				}
				preview.repaint() ;
			}

			double[] previewAngles() {
				if (tableModel.frames.isEmpty()) return new double[6] ;
				int targetIndex = Math.max(0,Math.min(animationFrame,tableModel.frames.size()-1)) ;
				AnimationFrame target = tableModel.frames.get(targetIndex) ;
				if (!playing) return target.angles ;
				double[] source ;
				if (targetIndex > 0) source = tableModel.frames.get(targetIndex-1).angles ;
				else if (action.getSelectedIndex() > 0) source = frame.model.animationFrame((String)profile.getSelectedItem(),0).angles ;
				else source = tableModel.frames.get(tableModel.frames.size()-1).angles ;
				double amount = Math.min(1.0,frameTime/Math.max(1.0,target.duration)) ;
				double[] result = new double[6] ;
				for (int index=0; index<result.length; index++) {
					double difference = target.angles[index]-source[index] ;
					while (difference > 180.0) difference -= 360.0 ;
					while (difference < -180.0) difference += 360.0 ;
					result[index] = source[index]+difference*amount ;
				}
				return result ;
			}

			class AnimationPreviewPanel extends JPanel {
				AnimationPreviewPanel() {setPreferredSize(new Dimension(700,250)); setBackground(new Color(232,235,236));}
				protected void paintComponent(Graphics graphics) {
					super.paintComponent(graphics) ;
					Graphics2D previewGraphics = (Graphics2D)graphics.create() ;
					try {
						previewGraphics.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON) ;
						previewGraphics.setColor(new Color(215,219,221)) ;
						for (int x=0; x<getWidth(); x+=32) previewGraphics.drawLine(x,0,x,getHeight()) ;
						for (int y=0; y<getHeight(); y+=32) previewGraphics.drawLine(0,y,getWidth(),y) ;
						frame.canvas.drawPosedWeapon(previewGraphics,weapon,previewAngles(),decimal(spriteOffsetX),decimal(spriteOffsetY),getWidth()/2.0,getHeight()/2.0+25.0,5.0,false) ;
						String status = tableModel.frames.isEmpty() ? "NO KEYFRAMES" : ((playing ? "PLAYING" : "KEYFRAME")+"  "+(animationFrame+1)+" / "+tableModel.frames.size()+"  "+(int)frameTime+" ms") ;
						frame.canvas.drawCentered(previewGraphics,blockName()+"  |  "+status,getWidth()/2,24,new Color(70,76,82)) ;
					} finally {previewGraphics.dispose() ;}
				}
			}

			String blockName() {
				String selected = (String)profile.getSelectedItem() ;
				if (selected == null) return null ;
				return selected + (action.getSelectedIndex() == 1 ? "_FIRE" : action.getSelectedIndex() == 2 ? "_RELOAD" : "") ;
			}

			void addProfile() {
				String name = JOptionPane.showInputDialog(this,"New animation profile name:","Add animation profile",JOptionPane.PLAIN_MESSAGE) ;
				if (name == null) return ;
				try {
					frame.model.addAnimationProfile(name) ;
					String added = name.trim().toUpperCase() ;
					profile.addItem(added) ; animationType.addItem(added) ; profile.setSelectedItem(added) ;
				} catch (IOException exception) {JOptionPane.showMessageDialog(this,exception.getMessage(),"Cannot add profile",JOptionPane.ERROR_MESSAGE) ;}
			}

			void duplicateProfile() {
				String source = (String)profile.getSelectedItem() ;
				if (source == null) return ;
				String name = JOptionPane.showInputDialog(this,"Name for the copy of " + source + ":","Duplicate animation profile",JOptionPane.PLAIN_MESSAGE) ;
				if (name == null) return ;
				try {
					frame.model.duplicateAnimationProfile(source,name) ;
					String added = name.trim().toUpperCase() ;
					profile.addItem(added) ; animationType.addItem(added) ; profile.setSelectedItem(added) ;
				} catch (IOException exception) {JOptionPane.showMessageDialog(this,exception.getMessage(),"Cannot duplicate profile",JOptionPane.ERROR_MESSAGE) ;}
			}

			void deleteProfile() {
				String selected = (String)profile.getSelectedItem() ;
				if (selected == null) return ;
				if ("PRIMARY".equals(selected) || "SECONDARY".equals(selected)) {
					JOptionPane.showMessageDialog(this,"Built-in animation profiles cannot be removed.","Cannot delete profile",JOptionPane.ERROR_MESSAGE) ; return ;
				}
				if (JOptionPane.showConfirmDialog(this,"Delete "+selected+" and clear every weapon assignment using it?","Delete animation profile",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) != JOptionPane.YES_OPTION) return ;
				try {
					frame.model.deleteAnimationProfile(selected) ;
					profile.removeItem(selected) ; animationType.removeItem(selected) ;
					String assigned = frame.model.weaponAnimationProfile(weapon.id) ;
					animationType.setSelectedItem(assigned == null ? "Inventory default" : assigned) ;
					tableModel.selectBlock() ; restartPreview() ; weaponPosePreview.repaint() ; frame.canvas.repaint() ;
				} catch (IOException exception) {JOptionPane.showMessageDialog(this,exception.getMessage(),"Cannot delete profile",JOptionPane.ERROR_MESSAGE) ;}
			}

			void saveAnimations() {
				if (table.isEditing()) table.getCellEditor().stopCellEditing() ;
				try {
					frame.model.saveAnimations() ;
					weaponPosePreview.repaint() ; frame.canvas.repaint() ;
					JOptionPane.showMessageDialog(this,"Saved animations.txt. Restart the game to load animation changes.","Animations saved",JOptionPane.INFORMATION_MESSAGE) ;
				} catch (IOException exception) {JOptionPane.showMessageDialog(this,exception.getMessage(),"Save failed",JOptionPane.ERROR_MESSAGE) ;}
			}

			class AnimationTableModel extends AbstractTableModel {
				final String[] columns = {"Duration ms","Body","Right arm","Right hand","Left arm","Left hand","Gun"} ;
				List<AnimationFrame> frames = new ArrayList<AnimationFrame>() ;
				void selectBlock() {
					String block = blockName() ;
					frames = block == null ? new ArrayList<AnimationFrame>() : frame.model.animations.get(block) ;
					if (frames == null) frames = new ArrayList<AnimationFrame>() ;
					fireTableDataChanged() ;
				}
				public int getRowCount() {return frames.size() ;}
				public int getColumnCount() {return columns.length ;}
				public String getColumnName(int column) {return columns[column] ;}
				public Class<?> getColumnClass(int column) {return column == 0 ? Integer.class : Double.class ;}
				public boolean isCellEditable(int row, int column) {return true ;}
				public Object getValueAt(int row, int column) {AnimationFrame value=frames.get(row); return column == 0 ? Integer.valueOf(value.duration) : Double.valueOf(value.angles[column-1]) ;}
				public void setValueAt(Object value, int row, int column) {
					try {
						AnimationFrame target=frames.get(row) ;
						if (column == 0) target.duration=Math.max(1,Integer.parseInt(value.toString())) ;
						else target.angles[column-1]=Double.parseDouble(value.toString()) ;
						fireTableCellUpdated(row,column) ; weaponPosePreview.repaint() ; preview.repaint() ;
					} catch (NumberFormatException ignored) {fireTableCellUpdated(row,column) ;}
				}
				void addFrame() {
					AnimationFrame added = frames.isEmpty() ? new AnimationFrame(150,new double[6]) : frames.get(frames.size()-1).copy() ;
					frames.add(added) ; fireTableRowsInserted(frames.size()-1,frames.size()-1) ; restartPreview() ;
				}
				void removeFrame(int row) {
					if (row < 0 || frames.size() <= 1) return ;
					frames.remove(row) ; fireTableRowsDeleted(row,row) ; restartPreview() ;
				}
				void copyFrame(int row) {
					if (row < 0 || row >= frames.size()) return ;
					copiedFrame = frames.get(row).copy() ;
				}
				void pasteFrame(int row) {
					if (copiedFrame == null) return ;
					int insertion = row < 0 ? frames.size() : Math.min(frames.size(),row+1) ;
					frames.add(insertion,copiedFrame.copy()) ; fireTableRowsInserted(insertion,insertion) ;
					table.setRowSelectionInterval(insertion,insertion) ;
				}
			}
		}

		JPanel previewZoomBar(final JSpinner zoom, final JComponent preview) {
			JPanel bar = new JPanel(new FlowLayout(FlowLayout.LEFT, 6, 2)) ;
			bar.add(new JLabel("Zoom")) ;
			zoom.setPreferredSize(new Dimension(54, 24)) ;
			zoom.setToolTipText("Preview zoom from 1x to 8x") ;
			bar.add(zoom) ;
			bar.add(new JLabel("x")) ;
			JLabel hint = new JLabel("Scroll over preview") ;
			hint.setForeground(MUTED) ;
			bar.add(hint) ;
			zoom.addChangeListener(event -> preview.repaint()) ;
			preview.addMouseWheelListener(event -> {
				int value = integer(zoom)-event.getWheelRotation() ;
				zoom.setValue(Integer.valueOf(Math.max(1,Math.min(8,value)))) ;
				event.consume() ;
			}) ;
			return bar ;
		}

		JPanel formPanel() {
			JPanel panel = new JPanel(new GridLayout(0, 2, 14, 9)) ;
			panel.setBorder(new EmptyBorder(4, 4, 4, 8)) ;
			return panel ;
		}

		JScrollPane scrollable(JPanel panel) {
			JScrollPane scroll = new JScrollPane(panel, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER) ;
			scroll.setBorder(null) ;
			scroll.getVerticalScrollBar().setUnitIncrement(16) ;
			return scroll ;
		}

		void add(JPanel panel, String label, JComponent control) {
			JLabel component = new JLabel(label) ;
			component.setToolTipText(fieldHelp(label)) ;
			control.setToolTipText(fieldHelp(label)) ;
			panel.add(component) ;
			panel.add(control) ;
		}

		String fieldHelp(String label) {
			if (label.startsWith("Magazine count")) return "Total magazines, including the loaded magazine" ;
			if (label.startsWith("Shot delay")) return "Minimum milliseconds between shots; lower is faster" ;
			if (label.startsWith("View angle")) return "View-cone half-angle; 0 disables the effect" ;
			if (label.startsWith("Launch origin")) return "Standard fires from the normal weapon position; Shoulder uses the shoulder-mounted position" ;
			if (label.startsWith("Sprite X")) return "Moves the weapon along its aiming direction; also moves its muzzle and shot origin" ;
			if (label.startsWith("Sprite Y")) return "Moves the weapon sideways from the player; also moves its muzzle and shot origin" ;
			if (label.startsWith("Muzzle forward")) return "Moves the muzzle flash from the animated gun anchor along the barrel" ;
			if (label.startsWith("Muzzle sideways")) return "Moves the muzzle flash perpendicular to the barrel" ;
			return label ;
		}

		void loadValues() {
			name.setText(weapon.name) ; damage.setValue(weapon.damage) ; delay.setValue(weapon.delay) ;
			spread.setValue((double)weapon.spread) ; clip.setValue(weapon.clip) ; numClips.setValue(weapon.numClips) ;
			reloadDelay.setValue(weapon.reloadDelay) ; walkingSpeed.setValue((double)weapon.walkingSpeed) ;
			bulletSpeed.setValue((double)weapon.bulletSpeed) ; viewAngle.setValue((double)weapon.viewAngle) ; cost.setValue(weapon.cost) ;
			selectIndex(type,weapon.type) ; selectIndex(fireMode,weapon.fireMode) ; pellets.setValue(weapon.pellets) ;
			animationType.removeAllItems() ;
			animationType.addItem("Inventory default") ;
			for (String profile : frame.model.animationProfiles()) animationType.addItem(profile) ;
			String assignedProfile = frame.model.weaponAnimationProfile(weapon.id) ;
			if (assignedProfile != null) animationType.setSelectedItem(assignedProfile) ;
			selectIndex(scope,weapon.scope) ; selectIndex(category,weapon.category) ; selectIndex(teams,weapon.teams) ;
			flashStyle.setValue(weapon.flashStyle) ;
			selectIndex(projectileType,weapon.projectileType) ; projectileStyle.setValue(weapon.projectileStyle) ;
			selectIndex(projectileLaunchOrigin,frame.model.projectileLaunchOrigin(weapon)) ;
			explosionStyle.setValue(weapon.explosionStyle) ; explosionFrameTime.setValue((int)weapon.explosionFrameTime) ;
			Float radius = frame.model.projectileExplosionRadius(weapon.id) ;
			overrideProjectileExplosionRadius.setSelected(radius != null) ;
			projectileExplosionRadius.setValue(Double.valueOf(radius == null ? 200.0 : radius.doubleValue())) ;
			spriteOffsetX.setValue((double)weapon.spriteOffsetX) ; spriteOffsetY.setValue((double)weapon.spriteOffsetY) ;
			float[] muzzle = frame.model.muzzlePosition(weapon.id) ;
			muzzleForward.setValue(Double.valueOf(muzzle[0])) ; muzzleSideways.setValue(Double.valueOf(muzzle[1])) ;
			impactStyle.setValue(weapon.impactStyle) ; impactScale.setValue((double)weapon.impactScale) ;
			impactRed.setValue(weapon.impactRed) ; impactGreen.setValue(weapon.impactGreen) ; impactBlue.setValue(weapon.impactBlue) ;
			impactFadeTime.setValue(weapon.impactFadeTime) ;
			tracerStyle.setSelectedItem(tracer.style) ;
			tracerRed.setValue(tracer.red) ; tracerGreen.setValue(tracer.green) ; tracerBlue.setValue(tracer.blue) ;
			tracerAlpha.setValue(tracer.alpha) ; tracerLength.setValue((double)tracer.length) ; tracerWidth.setValue((double)tracer.width) ;
			laserEnabled.setSelected(laser.enabled) ; laserFalloffEnabled.setSelected(laser.falloffEnabled) ;
			laserRed.setValue(laser.red) ; laserGreen.setValue(laser.green) ; laserBlue.setValue(laser.blue) ; laserAlpha.setValue(laser.alpha) ;
			laserWidth.setValue((double)laser.width) ; laserRange.setValue((double)laser.range) ; laserFalloff.setValue((double)laser.falloff) ;
			laserEndDot.setSelected(laser.endDot) ; laserEndDotScale.setValue((double)laser.endDotScale) ;
			laserOffsetX.setValue((double)laser.offsetX) ; laserOffsetY.setValue((double)laser.offsetY) ;
			fireShakeEnabled.setSelected(screenShake.fireEnabled) ; fireShakeMagnitude.setValue(screenShake.fireMagnitude) ;
			fireShakeTime.setValue(screenShake.fireTime) ; explosionShakeEnabled.setSelected(screenShake.explosionEnabled) ;
			explosionShakeMagnitude.setValue(screenShake.explosionMagnitude) ; explosionShakeTime.setValue(screenShake.explosionTime) ;
			explosionShakeRadius.setValue((double)screenShake.explosionRadius) ;
			refreshTracerPreview() ;
			refreshLaserPreview() ;
			impactPreview.repaint() ;
			refreshProjectileControls() ;
		}

		class WeaponPosePreviewPanel extends JPanel {
			double[] gunAnchor = {0.0,0.0,0.0} ;
			double muzzleX, muzzleY ;
			boolean draggingMuzzle ;
			WeaponPosePreviewPanel() {
				setPreferredSize(new Dimension(560, 190)) ; setBackground(new Color(232, 235, 236)) ;
				setCursor(Cursor.getPredefinedCursor(Cursor.CROSSHAIR_CURSOR)) ;
				MouseAdapter drag = new MouseAdapter() {
					public void mousePressed(MouseEvent event) {
						draggingMuzzle = Point2D.distance(event.getX(),event.getY(),muzzleX,muzzleY) <= 18.0 ;
						if (draggingMuzzle) updateMuzzle(event) ;
					}
					public void mouseDragged(MouseEvent event) {if (draggingMuzzle) updateMuzzle(event) ;}
					public void mouseReleased(MouseEvent event) {draggingMuzzle = false ;}
				} ;
				addMouseListener(drag) ; addMouseMotionListener(drag) ;
			}
			void updateMuzzle(MouseEvent event) {
				double dx = (event.getX()-gunAnchor[0])/4.0, dy = (event.getY()-gunAnchor[1])/4.0 ;
				double directionX = Math.cos(gunAnchor[2]), directionY = Math.sin(gunAnchor[2]) ;
				muzzleForward.setValue(Double.valueOf(Math.max(-100.0,Math.min(100.0,dx*directionX+dy*directionY)))) ;
				muzzleSideways.setValue(Double.valueOf(Math.max(-100.0,Math.min(100.0,-dx*directionY+dy*directionX)))) ;
			}
			protected void paintComponent(Graphics graphics) {
				super.paintComponent(graphics) ;
				Graphics2D g = (Graphics2D)graphics.create() ;
				try {
					g.setColor(new Color(215, 219, 221)) ;
					for (int x=0 ; x<getWidth() ; x+=32) g.drawLine(x,0,x,getHeight()) ;
					for (int y=0 ; y<getHeight() ; y+=32) g.drawLine(0,y,getWidth(),y) ;
					g.setColor(new Color(184, 190, 194)) ;
					g.drawOval(getWidth()/2-48,getHeight()/2-36,96,96) ;
					String poseName = animationType.getSelectedIndex() <= 0 ? new String[] {"PRIMARY","SECONDARY","KNIFE","GRENADE"}[type.getSelectedIndex()] : (String)animationType.getSelectedItem() ;
					double zoom = 4.0 ;
					gunAnchor = frame.canvas.drawPosedWeapon(g, weapon, poseName, decimal(spriteOffsetX), decimal(spriteOffsetY), getWidth()/2.0, getHeight()/2.0+12.0, zoom) ;
					double directionX = Math.cos(gunAnchor[2]), directionY = Math.sin(gunAnchor[2]) ;
					muzzleX = gunAnchor[0]+(directionX*decimal(muzzleForward)-directionY*decimal(muzzleSideways))*zoom ;
					muzzleY = gunAnchor[1]+(directionY*decimal(muzzleForward)+directionX*decimal(muzzleSideways))*zoom ;
					if (frame.canvas.hasMuzzleFlash(weapon)) {
						BufferedImage flash = frame.model.flash(integer(flashStyle)) ;
						frame.canvas.drawQuad(g,flash,frame.flashFrame()*32,0,32,32,muzzleX,muzzleY,16,-16,gunAnchor[2]-Math.PI/2.0,zoom,false) ;
					}
					g.setColor(new Color(225,83,52)) ;
					g.drawLine((int)muzzleX-7,(int)muzzleY,(int)muzzleX+7,(int)muzzleY) ;
					g.drawLine((int)muzzleX,(int)muzzleY-7,(int)muzzleX,(int)muzzleY+7) ;
					g.drawOval((int)muzzleX-4,(int)muzzleY-4,8,8) ;
					frame.canvas.drawCentered(g, "POSE: " + poseName + "  |  MUZZLE " + decimal(muzzleForward) + ", " + decimal(muzzleSideways), getWidth()/2, 20, new Color(70,76,82)) ;
				} finally {g.dispose() ;}
			}
		}

		void selectIndex(JComboBox<String> comboBox, int index) {
			comboBox.setSelectedIndex(index >= 0 && index < comboBox.getItemCount() ? index : 0) ;
		}

		void refreshProjectileControls() {
			boolean rocket = projectileType.getSelectedIndex() == 1 ;
			projectileStyle.setEnabled(rocket) ; explosionStyle.setEnabled(rocket) ; explosionFrameTime.setEnabled(rocket) ;
			overrideProjectileExplosionRadius.setEnabled(rocket) ;
			projectileExplosionRadius.setEnabled(rocket && overrideProjectileExplosionRadius.isSelected()) ;
			int style = integer(explosionStyle) ;
			boolean available = frame.model.explosionAtlas(style) != null ;
			explosionDuration.setText((integer(explosionFrameTime)*16) + " ms" + (available ? "" : " - MISSING " + frame.model.explosionAtlasName(style))) ;
			explosionDuration.setForeground(available ? UIManager.getColor("Label.foreground") : new Color(220,70,60)) ;
		}

		class ProjectilePreviewPanel extends JPanel {
			ProjectilePreviewPanel() {setPreferredSize(new Dimension(560,300)) ; setBackground(BACKGROUND) ;}
			protected void paintComponent(Graphics graphics) {
				super.paintComponent(graphics) ;
				Graphics2D g=(Graphics2D)graphics.create() ;
				try {
					g.setRenderingHint(RenderingHints.KEY_INTERPOLATION,RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR) ;
					if (projectileType.getSelectedIndex() != 1) {frame.canvas.drawCentered(g,"Bullet projectile",getWidth()/2,getHeight()/2,MUTED) ; return ;}
					int style=integer(projectileStyle), column=style%4, row=style/4 ;
					int scale=Math.max(2,Math.min(6,getWidth()/150)) ;
					g.drawImage(frame.model.projectileAtlas,getWidth()/4-16*scale,getHeight()/2-16*scale,getWidth()/4+16*scale,getHeight()/2+16*scale,column*32,row*32,column*32+32,row*32+32,null) ;
					int explosionFrame=(int)((System.currentTimeMillis()/integer(explosionFrameTime))%16) ;
					int explosionColumn=explosionFrame%4, explosionRow=explosionFrame/4 ;
					BufferedImage explosion=frame.model.explosionAtlas(integer(explosionStyle)) ;
					if (explosion == null) {
						frame.canvas.drawCentered(g,"MISSING",getWidth()*3/4,getHeight()/2-8,new Color(220,70,60)) ;
						frame.canvas.drawCentered(g,frame.model.explosionAtlasName(integer(explosionStyle)),getWidth()*3/4,getHeight()/2+18,MUTED) ;
						return ;
					}
					g.drawImage(explosion,getWidth()*3/4-16*scale,getHeight()/2-16*scale,getWidth()*3/4+16*scale,getHeight()/2+16*scale,explosionColumn*32,explosionRow*32,explosionColumn*32+32,explosionRow*32+32,null) ;
					frame.canvas.drawCentered(g,"PROJECTILE " + style,getWidth()/4,32,MUTED) ;
					frame.canvas.drawCentered(g,"EXPLOSION FRAMES",getWidth()*3/4,32,MUTED) ;
				} finally {g.dispose() ;}
			}
		}

		void installImpactPreviewListeners() {
			impactStyle.addChangeListener(event -> impactPreview.repaint()) ;
			impactScale.addChangeListener(event -> impactPreview.repaint()) ;
			impactLightBackground.addActionListener(event -> impactPreview.repaint()) ;
			JSpinner[] controls = {impactRed, impactGreen, impactBlue, impactFadeTime} ;
			for (JSpinner control : controls) control.addChangeListener(event -> {impactColorSwatch.repaint() ; impactPreview.repaint() ;}) ;
		}

		Color impactColor() {return new Color(integer(impactRed),integer(impactGreen),integer(impactBlue)) ;}

		void chooseImpactColor() {
			Color selected = JColorChooser.showDialog(this,"Choose impact color",impactColor()) ;
			if (selected == null) return ;
			impactRed.setValue(selected.getRed()) ; impactGreen.setValue(selected.getGreen()) ; impactBlue.setValue(selected.getBlue()) ;
		}

		class ImpactColorSwatch extends JComponent {
			ImpactColorSwatch() {setPreferredSize(new Dimension(64,64)) ; setToolTipText("Current impact color") ;}
			protected void paintComponent(Graphics graphics) {
				Graphics2D g = (Graphics2D)graphics.create() ;
				try {g.setColor(impactColor()) ; g.fillOval(4,4,getWidth()-8,getHeight()-8) ;
					g.setColor(new Color(75,80,86)) ; g.setStroke(new BasicStroke(2.0f)) ; g.drawOval(4,4,getWidth()-8,getHeight()-8) ;}
				finally {g.dispose() ;}
			}
		}

		class ImpactPreviewPanel extends JPanel {
			ImpactPreviewPanel() {setPreferredSize(new Dimension(430, 180)) ; setBackground(BACKGROUND) ;}
			protected void paintComponent(Graphics graphics) {
				super.paintComponent(graphics) ;
				Graphics2D g = (Graphics2D)graphics.create() ;
				try {
					Color checkerDark = impactLightBackground.isSelected() ? new Color(220,224,228) : PANEL ;
					Color checkerLight = impactLightBackground.isSelected() ? new Color(248,249,250) : PANEL_LIGHT ;
					int tile = 12 ;
					for (int y=0; y<getHeight(); y+=tile) for (int x=0; x<getWidth(); x+=tile) {
						g.setColor(((x/tile+y/tile)&1) == 0 ? checkerDark : checkerLight) ;
						g.fillRect(x,y,tile,tile) ;
					}
					int style = integer(impactStyle) ;
					BufferedImage atlas = frame.model.impactPage(style) ;
					Rectangle cell = frame.model.impactCell(style) ;
					if (atlas == null || cell == null) return ;
					int fadeTime = integer(impactFadeTime) ;
					float progress = Math.min(1.0f,(System.currentTimeMillis()%(fadeTime+400))/(float)fadeTime) ;
					int size = Math.max(3,(int)Math.round(32.0*decimal(impactScale)*(0.3-0.2*progress))) ;
					int x = (getWidth()-size)/2, y = (getHeight()-size)/2 ;
					BufferedImage source = new BufferedImage(32,32,BufferedImage.TYPE_INT_ARGB) ;
					Graphics2D sourceGraphics = source.createGraphics() ;
					try {sourceGraphics.drawImage(atlas,0,0,32,32,cell.x,cell.y,cell.x+32,cell.y+32,null) ;}
					finally {sourceGraphics.dispose() ;}
					Color color = impactColor() ;
					RescaleOp tint = new RescaleOp(new float[] {color.getRed()/255.0f,color.getGreen()/255.0f,color.getBlue()/255.0f,1.0f},new float[4],null) ;
					BufferedImage tinted = tint.filter(source,null) ;
					g.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR) ;
					g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER,1.0f-progress)) ;
					g.drawImage(tinted,x,y,size,size,null) ;
				} finally {g.dispose() ;}
			}
		}

		void installLaserPreviewListeners() {
			laserEnabled.addActionListener(event -> refreshLaserPreview()) ;
			laserFalloffEnabled.addActionListener(event -> refreshLaserPreview()) ;
			laserEndDot.addActionListener(event -> refreshLaserPreview()) ;
			JSpinner[] controls = {laserRed, laserGreen, laserBlue, laserAlpha, laserWidth, laserRange, laserFalloff, laserEndDotScale, laserOffsetX, laserOffsetY} ;
			for (JSpinner control : controls) control.addChangeListener(event -> refreshLaserPreview()) ;
		}

		LaserConfig previewLaser() {
			LaserConfig preview = new LaserConfig() ;
			preview.enabled = laserEnabled.isSelected() ; preview.falloffEnabled = laserFalloffEnabled.isSelected() ;
			preview.red = integer(laserRed) ; preview.green = integer(laserGreen) ; preview.blue = integer(laserBlue) ; preview.alpha = integer(laserAlpha) ;
			preview.width = decimal(laserWidth) ; preview.range = decimal(laserRange) ; preview.falloff = decimal(laserFalloff) ;
			preview.endDot = laserEndDot.isSelected() ; preview.endDotScale = decimal(laserEndDotScale) ;
			preview.offsetX = decimal(laserOffsetX) ; preview.offsetY = decimal(laserOffsetY) ;
			return preview ;
		}

		void refreshLaserPreview() {
			laserFalloff.setEnabled(laserFalloffEnabled.isSelected()) ;
			laserEndDotScale.setEnabled(laserEndDot.isSelected()) ;
			laserColorSwatch.repaint() ;
			laserPreview.repaint() ;
		}

		void chooseLaserColor() {
			Color selected = JColorChooser.showDialog(this, "Choose laser color", previewLaser().color()) ;
			if (selected == null) return ;
			laserRed.setValue(selected.getRed()) ; laserGreen.setValue(selected.getGreen()) ; laserBlue.setValue(selected.getBlue()) ;
			refreshLaserPreview() ;
		}

		class LaserColorSwatch extends JComponent {
			LaserColorSwatch() {setPreferredSize(new Dimension(64, 64)) ; setToolTipText("Current laser RGBA color") ;}
			protected void paintComponent(Graphics graphics) {
				Graphics2D g = (Graphics2D)graphics.create() ;
				try {g.setColor(previewLaser().color()) ; g.fillOval(4, 4, getWidth()-8, getHeight()-8) ;
					g.setColor(new Color(75, 80, 86)) ; g.setStroke(new BasicStroke(2.0f)) ; g.drawOval(4, 4, getWidth()-8, getHeight()-8) ;}
				finally {g.dispose() ;}
			}
		}

		class LaserPreviewPanel extends JPanel {
			double emitterX, emitterY ;
			boolean draggingEmitter ;
			LaserPreviewPanel() {
				setPreferredSize(new Dimension(430, 110)) ; setBackground(BACKGROUND) ;
				setCursor(Cursor.getPredefinedCursor(Cursor.CROSSHAIR_CURSOR)) ;
				MouseAdapter drag = new MouseAdapter() {
					public void mousePressed(MouseEvent event) {
						draggingEmitter = Point2D.distance(event.getX(),event.getY(),emitterX,emitterY) <= 18.0 ;
						if (draggingEmitter) updateEmitter(event) ;
					}
					public void mouseDragged(MouseEvent event) {if (draggingEmitter) updateEmitter(event) ;}
					public void mouseReleased(MouseEvent event) {draggingEmitter = false ;}
				} ;
				addMouseListener(drag) ; addMouseMotionListener(drag) ;
			}
			void updateEmitter(MouseEvent event) {
				double zoom = integer(laserPreviewZoom) ;
				laserOffsetX.setValue(Double.valueOf(Math.max(-100.0,Math.min(100.0,(event.getX()-42.0)/zoom)))) ;
				laserOffsetY.setValue(Double.valueOf(Math.max(-100.0,Math.min(100.0,(event.getY()-getHeight()/2.0)/zoom)))) ;
			}
			protected void paintComponent(Graphics graphics) {
				super.paintComponent(graphics) ;
				Graphics2D g = (Graphics2D)graphics.create() ;
				try {
					g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON) ;
					LaserConfig preview = previewLaser() ;
					if (!preview.enabled) {frame.canvas.drawCentered(g, "disabled", getWidth()/2, getHeight()/2, MUTED) ; return ;}
					double zoom = integer(laserPreviewZoom) ;
					double originX = 42.0, originY = getHeight()/2.0 ;
					g.setColor(new Color(110, 116, 124)) ;
					g.drawLine((int)originX-5, (int)originY, (int)originX+5, (int)originY) ;
					g.drawLine((int)originX, (int)originY-5, (int)originX, (int)originY+5) ;
					emitterX = originX+preview.offsetX*zoom ; emitterY = originY+preview.offsetY*zoom ;
					double endX = Math.min(getWidth()-18.0, emitterX+Math.max(20.0, preview.range*0.225*zoom)) ;
					int segments = preview.falloffEnabled && preview.falloff > 0.0f ? 12 : 1 ;
					for (int index=0 ; index<segments ; index++) {
						double from = index/(double)segments, to = (index+1)/(double)segments ;
						double intensity = segments == 1 ? 1.0 : Math.pow(Math.max(0.0, 1.0-(from+to)*0.5), preview.falloff) ;
						Color color = new Color(preview.red, preview.green, preview.blue, (int)(preview.alpha*intensity)) ;
						frame.canvas.drawTracerLine(g,emitterX+(endX-emitterX)*from,emitterY,emitterX+(endX-emitterX)*to,emitterY,preview.width*(float)zoom/2.0f,color) ;
					}
					if (preview.endDot) {
						double intensity = preview.falloffEnabled && preview.falloff > 0.0f ? 0.15 : 1.0 ;
						frame.canvas.drawTracerPoint(g,endX,emitterY,preview.endDotScale*(float)zoom/2.0f,new Color(preview.red,preview.green,preview.blue,(int)(preview.alpha*intensity))) ;
					}
					g.setColor(ACCENT) ; g.fillOval((int)emitterX-5,(int)emitterY-5,10,10) ;
					g.setColor(Color.WHITE) ; g.drawOval((int)emitterX-7,(int)emitterY-7,14,14) ;
					double[] gunAnchor = frame.canvas.idleGunAnchor(weapon) ;
					frame.canvas.drawWeaponCell(g,frame.model.handPage(weapon.id),weapon.id,
					                            originX+gunAnchor[0]*zoom,originY+gunAnchor[1]*zoom,
					                            16,8,-Math.PI/2.0,zoom) ;
				} finally {g.dispose() ;}
			}
		}

		void installTracerPreviewListeners() {
			tracerStyle.addActionListener(event -> refreshTracerPreview()) ;
			tracerLightBackground.addActionListener(event -> refreshTracerPreview()) ;
			JSpinner[] controls = {tracerRed, tracerGreen, tracerBlue, tracerAlpha, tracerLength, tracerWidth} ;
			for (JSpinner control : controls) control.addChangeListener(event -> refreshTracerPreview()) ;
		}

		TracerConfig previewTracer() {
			TracerConfig preview = new TracerConfig() ;
			preview.style = (String)tracerStyle.getSelectedItem() ;
			preview.red = integer(tracerRed) ; preview.green = integer(tracerGreen) ; preview.blue = integer(tracerBlue) ;
			preview.alpha = integer(tracerAlpha) ; preview.length = decimal(tracerLength) ; preview.width = decimal(tracerWidth) ;
			return preview ;
		}

		void refreshTracerPreview() {
			tracerColorSwatch.repaint() ;
			tracerPreview.repaint() ;
		}

		void chooseTracerColor() {
			Color selected = JColorChooser.showDialog(this, "Choose tracer color", previewTracer().color()) ;
			if (selected == null) return ;
			tracerRed.setValue(selected.getRed()) ;
			tracerGreen.setValue(selected.getGreen()) ;
			tracerBlue.setValue(selected.getBlue()) ;
			refreshTracerPreview() ;
		}

		class ColorSwatch extends JComponent {
			ColorSwatch() {
				setPreferredSize(new Dimension(64, 64)) ;
				setMinimumSize(new Dimension(64, 64)) ;
				setToolTipText("Current tracer RGBA color") ;
			}

			protected void paintComponent(Graphics graphics) {
				Graphics2D g = (Graphics2D)graphics.create() ;
				try {
					g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON) ;
					int size = Math.min(getWidth(), getHeight()) - 8 ;
					int x = (getWidth() - size) / 2 ;
					int y = (getHeight() - size) / 2 ;
					Shape circle = new java.awt.geom.Ellipse2D.Double(x, y, size, size) ;
					Shape oldClip = g.getClip() ;
					g.clip(circle) ;
					int cell = 8 ;
					for (int row = y ; row < y + size ; row += cell) {
						for (int column = x ; column < x + size ; column += cell) {
							g.setColor(((row-y)/cell + (column-x)/cell) % 2 == 0 ? Color.WHITE : new Color(190, 194, 198)) ;
							g.fillRect(column, row, cell, cell) ;
						}
					}
					g.setColor(previewTracer().color()) ;
					g.fill(circle) ;
					g.setClip(oldClip) ;
					g.setColor(new Color(75, 80, 86)) ;
					g.setStroke(new BasicStroke(2.0f)) ;
					g.draw(circle) ;
				} finally {g.dispose() ;}
			}
		}

		class TracerPreviewPanel extends JPanel {
			TracerPreviewPanel() {
				setPreferredSize(new Dimension(430, 110)) ;
				setMinimumSize(new Dimension(280, 110)) ;
				setBackground(BACKGROUND) ;
			}

			protected void paintComponent(Graphics graphics) {
				super.paintComponent(graphics) ;
				Graphics2D g = (Graphics2D)graphics.create() ;
				try {
					g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON) ;
					g.setColor(tracerLightBackground.isSelected() ? new Color(242,244,246) : BACKGROUND) ;
					g.fillRect(0,0,getWidth(),getHeight()) ;
					g.setColor(tracerLightBackground.isSelected() ? new Color(210,214,218) : new Color(25,28,32)) ;
					for (int x = 0 ; x < getWidth() ; x += 20) g.drawLine(x, 0, x, getHeight()) ;
					for (int y = 0 ; y < getHeight() ; y += 20) g.drawLine(0, y, getWidth(), y) ;
					Rectangle bounds = new Rectangle(8, 8, Math.max(1, getWidth()-16), Math.max(1, getHeight()-16)) ;
					frame.canvas.drawTracerCentered(g, previewTracer(), weapon, bounds, 0.0, integer(tracerPreviewZoom)) ;
				} finally {g.dispose() ;}
			}
		}

		int integer(JSpinner spinner) {return ((Number)spinner.getValue()).intValue() ;}
		float decimal(JSpinner spinner) {return ((Number)spinner.getValue()).floatValue() ;}

		void importImpactSprite() {
			int style = integer(impactStyle) ;
			JFileChooser chooser = new JFileChooser() ;
			chooser.setDialogTitle("Select a 32x32 PNG for impact style " + style) ;
			chooser.setFileFilter(new FileNameExtensionFilter("PNG image (32x32)","png")) ;
			chooser.setAcceptAllFileFilterUsed(false) ;
			if (chooser.showOpenDialog(this) != JFileChooser.APPROVE_OPTION) return ;
			try {
				frame.model.replaceImpactSprite(style,chooser.getSelectedFile()) ;
				impactPreview.repaint() ;
				String atlas = "bulletimpacts" + (style < 64 ? "" : "2") + ".png" ;
				JOptionPane.showMessageDialog(this,"Updated " + atlas + " for impact style " + style + ".\nA .bak copy was created when replacing an existing atlas.",
				                              "Impact sprite updated",JOptionPane.INFORMATION_MESSAGE) ;
			} catch (IOException exception) {
				JOptionPane.showMessageDialog(this,exception.getMessage(),"Impact import failed",JOptionPane.ERROR_MESSAGE) ;
			}
		}

		void importSprite(boolean hand) {
			JFileChooser chooser = new JFileChooser() ;
			chooser.setDialogTitle("Select a 32x32 " + (hand ? "hand" : "ground") + " PNG") ;
			chooser.setFileFilter(new FileNameExtensionFilter("PNG image (32x32)", "png")) ;
			chooser.setAcceptAllFileFilterUsed(false) ;
			if (chooser.showOpenDialog(this) != JFileChooser.APPROVE_OPTION) return ;
			try {
				if (newWeapon) {
					BufferedImage image = ImageIO.read(chooser.getSelectedFile()) ;
					if (image == null || image.getWidth() != 32 || image.getHeight() != 32) {
						throw new IOException("Weapon sprites must be valid PNG images exactly 32x32 pixels.") ;
					}
					if (hand) pendingHandSprite = chooser.getSelectedFile() ;
					else pendingGroundSprite = chooser.getSelectedFile() ;
					JOptionPane.showMessageDialog(this, (hand ? "Hand" : "Ground") + " sprite selected. It will be written when Add gun is pressed.",
					                              "Sprite selected", JOptionPane.INFORMATION_MESSAGE) ;
					return ;
				}
				frame.model.replaceWeaponSprite(weapon.id, chooser.getSelectedFile(), hand) ;
				frame.canvas.repaint() ;
				String atlas = frame.model.atlasName(hand, weapon.id / 64) ;
				JOptionPane.showMessageDialog(this, "Updated " + atlas + " for weapon ID " + weapon.id + ".\nA .bak copy was created.",
				                              "Sprite updated", JOptionPane.INFORMATION_MESSAGE) ;
			} catch (IOException exception) {
				JOptionPane.showMessageDialog(this, exception.getMessage(), "Sprite import failed", JOptionPane.ERROR_MESSAGE) ;
			}
		}

		void save() {
			String weaponName = name.getText().trim() ;
			if (!weaponName.matches("[^\\s]{1,14}")) {
				JOptionPane.showMessageDialog(this, "Name must be 1-14 characters with no spaces.", "Invalid name", JOptionPane.ERROR_MESSAGE) ;
				return ;
			}
			if (newWeapon && (pendingHandSprite == null || pendingGroundSprite == null)) {
				JOptionPane.showMessageDialog(this, "Select both a hand PNG and a ground PNG before adding the gun.",
				                              "Sprites required", JOptionPane.ERROR_MESSAGE) ;
				return ;
			}
			weapon.name = weaponName ; weapon.damage = integer(damage) ; weapon.delay = integer(delay) ;
			weapon.spread = decimal(spread) ; weapon.clip = integer(clip) ; weapon.numClips = integer(numClips) ;
			weapon.reloadDelay = integer(reloadDelay) ; weapon.walkingSpeed = decimal(walkingSpeed) ;
			weapon.bulletSpeed = decimal(bulletSpeed) ; weapon.viewAngle = decimal(viewAngle) ; weapon.cost = integer(cost) ;
			weapon.type = type.getSelectedIndex() ; weapon.fireMode = fireMode.getSelectedIndex() ; weapon.pellets = integer(pellets) ;
			weapon.scope = scope.getSelectedIndex() ; weapon.category = category.getSelectedIndex() ;
			weapon.teams = teams.getSelectedIndex() ; weapon.flashStyle = integer(flashStyle) ;
			weapon.projectileType = projectileType.getSelectedIndex() ; weapon.projectileStyle = integer(projectileStyle) ;
			weapon.explosionStyle = integer(explosionStyle) ; weapon.explosionFrameTime = integer(explosionFrameTime) ;
			weapon.spriteOffsetX = decimal(spriteOffsetX) ; weapon.spriteOffsetY = decimal(spriteOffsetY) ;
			weapon.impactStyle = integer(impactStyle) ; weapon.impactScale = decimal(impactScale) ;
			weapon.impactRed = integer(impactRed) ; weapon.impactGreen = integer(impactGreen) ; weapon.impactBlue = integer(impactBlue) ;
			weapon.impactFadeTime = integer(impactFadeTime) ;
			tracer.style = (String)tracerStyle.getSelectedItem() ;
			tracer.red = integer(tracerRed) ; tracer.green = integer(tracerGreen) ; tracer.blue = integer(tracerBlue) ;
			tracer.alpha = integer(tracerAlpha) ; tracer.length = decimal(tracerLength) ; tracer.width = decimal(tracerWidth) ;
			laser.enabled = laserEnabled.isSelected() ; laser.falloffEnabled = laserFalloffEnabled.isSelected() ;
			laser.red = integer(laserRed) ; laser.green = integer(laserGreen) ; laser.blue = integer(laserBlue) ; laser.alpha = integer(laserAlpha) ;
			laser.width = decimal(laserWidth) ; laser.range = decimal(laserRange) ; laser.falloff = decimal(laserFalloff) ;
			laser.endDot = laserEndDot.isSelected() ; laser.endDotScale = decimal(laserEndDotScale) ;
			laser.offsetX = decimal(laserOffsetX) ; laser.offsetY = decimal(laserOffsetY) ;
			screenShake.fireEnabled = fireShakeEnabled.isSelected() ; screenShake.fireMagnitude = integer(fireShakeMagnitude) ;
			screenShake.fireTime = integer(fireShakeTime) ; screenShake.explosionEnabled = explosionShakeEnabled.isSelected() ;
			screenShake.explosionMagnitude = integer(explosionShakeMagnitude) ; screenShake.explosionTime = integer(explosionShakeTime) ;
			screenShake.explosionRadius = decimal(explosionShakeRadius) ;
			boolean appended = false ;
			boolean dataSaved = false ;
			try {
				if (newWeapon && !frame.model.weapons.contains(weapon)) {
					frame.model.weapons.add(weapon) ;
					appended = true ;
				}
				frame.model.saveWeapons() ;
				dataSaved = true ;
				if (newWeapon) {
					frame.model.replaceWeaponSprite(weapon.id, pendingHandSprite, true) ;
					frame.model.replaceWeaponSprite(weapon.id, pendingGroundSprite, false) ;
				}
				frame.model.saveTracer(weapon.id, tracer) ;
				frame.model.saveLaser(weapon.id, laser) ;
				frame.model.saveScreenShake(weapon.id, screenShake) ;
				String selectedProfile = animationType.getSelectedIndex() <= 0 ? null : (String)animationType.getSelectedItem() ;
				frame.model.saveWeaponAnimationProfile(weapon.id,selectedProfile) ;
				Float radius = overrideProjectileExplosionRadius.isSelected() ? Float.valueOf(decimal(projectileExplosionRadius)) : null ;
				frame.model.saveProjectileExplosionRadius(weapon.id,radius) ;
				frame.model.saveProjectileLaunchOrigin(weapon.id,projectileLaunchOrigin.getSelectedIndex()) ;
				frame.model.saveMuzzlePosition(weapon.id,decimal(muzzleForward),decimal(muzzleSideways)) ;
				frame.reloadResources(weapon.id) ;
				dispose() ;
			} catch (IOException exception) {
				if (appended && !dataSaved) frame.model.weapons.remove(weapon) ;
				JOptionPane.showMessageDialog(this, exception.getMessage(), "Save failed", JOptionPane.ERROR_MESSAGE) ;
			}
		}
	}

	static class PreviewFrame extends JFrame {
		final ResourceModel model = new ResourceModel() ;
		final JComboBox<String> previewBox = new JComboBox<String>(new String[] {"Guns", "Players"}) ;
		final JTextField weaponSearch = new JTextField() ;
		final DefaultListModel<WeaponInfo> weaponListModel = new DefaultListModel<WeaponInfo>() ;
		final JList<WeaponInfo> weaponList = new JList<WeaponInfo>(weaponListModel) ;
		final JComboBox<String> teamBox = new JComboBox<String>(new String[] {"Counter-Terrorist", "Terrorist"}) ;
		final JComboBox<String> modelBox = new JComboBox<String>(new String[] {"Model 1", "Model 2", "Model 3", "Model 4"}) ;
		final JSlider angleSlider = new JSlider(0, 359, 90) ;
		final JCheckBox lightPreviewBox = new JCheckBox("Light preview background", false) ;
		final JComboBox<String> frameBox = new JComboBox<String>(new String[] {"Frame 1", "Frame 2", "Frame 3"}) ;
		final JCheckBox animateBox = new JCheckBox("Animate muzzle flash", true) ;
		final JLabel status = new JLabel(" ") ;
		final JLabel simulationStatus = new JLabel(" ") ;
		final JProgressBar actionProgress = new JProgressBar(0, 1000) ;
		final PreviewCanvas canvas = new PreviewCanvas(this) ;
		final Timer timer ;
		final Timer simulationTimer ;
		JButton fireButton ;
		JButton reloadButton ;
		long flashUntil ;
		int animatedFrame ;
		int simulatedWeaponId = -1, magazineAmmo, reserveAmmo, shotsFired ;
		WeaponInfo activeWeapon ;
		long nextShotAt, reloadStartedAt, reloadUntil ;
		boolean triggerHeld, reloading ;
		final List<TracerShot> tracerShots = new ArrayList<TracerShot>() ;

		PreviewFrame() {
			super("CSPSP Weapon Configurator & Previewer") ;
			setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE) ;
			setMinimumSize(new Dimension(1100, 700)) ;
			setSize(1280, 800) ;
			setLocationRelativeTo(null) ;
			getContentPane().setBackground(BACKGROUND) ;
			getContentPane().setLayout(new BorderLayout()) ;
			getContentPane().add(buildSidebar(), BorderLayout.WEST) ;
			getContentPane().add(canvas, BorderLayout.CENTER) ;
			status.setOpaque(true) ;
			status.setBackground(new Color(16, 18, 21)) ;
			status.setForeground(MUTED) ;
			status.setBorder(new EmptyBorder(7, 12, 7, 12)) ;
			getContentPane().add(status, BorderLayout.SOUTH) ;
			installListeners() ;
			timer = new Timer(90, new ActionListener() {
				public void actionPerformed(ActionEvent event) {
					animatedFrame = (animatedFrame + 1) % 3 ;
					if (animateBox.isSelected()) flashUntil = System.currentTimeMillis() + 120 ;
					canvas.repaint() ;
				}
			}) ;
			timer.start() ;
			simulationTimer = new Timer(16, new ActionListener() {
				public void actionPerformed(ActionEvent event) {updateSimulation() ;}
			}) ;
			simulationTimer.start() ;
			reloadResources() ;
		}

		JComponent buildSidebar() {
			JPanel sidebar = new JPanel() ;
			sidebar.setBackground(PANEL) ;
			sidebar.setBorder(new EmptyBorder(24, 22, 24, 22)) ;
			sidebar.setPreferredSize(new Dimension(340, 1080)) ;
			sidebar.setLayout(new BoxLayout(sidebar, BoxLayout.Y_AXIS)) ;
			JLabel title = new JLabel("Gun Configurator & Previewer") ;
			title.setForeground(TEXT) ;
			title.setFont(new Font("Dialog", Font.BOLD, 20)) ;
			title.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			sidebar.add(title) ;
			JLabel subtitle = new JLabel("CSPSP Gun Configurator & Trace Editor") ;
			subtitle.setForeground(MUTED) ;
			subtitle.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			sidebar.add(subtitle) ;
			sidebar.add(Box.createVerticalStrut(24)) ;
			sidebar.add(sectionLabel("PREVIEW")) ;
			sidebar.add(Box.createVerticalStrut(8)) ;
			addControl(sidebar, "PREVIEW", previewBox) ;
			sidebar.add(buildWeaponSelector()) ;
			sidebar.add(Box.createVerticalStrut(15)) ;
			addControl(sidebar, "TEAM", teamBox) ;
			addControl(sidebar, "PLAYER", modelBox) ;
			configureSlider(angleSlider, 90, 45) ;
			addControl(sidebar, "AIM ANGLE", angleSlider) ;
			lightPreviewBox.setOpaque(false) ;
			lightPreviewBox.setForeground(TEXT) ;
			lightPreviewBox.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			sidebar.add(lightPreviewBox) ;
			sidebar.add(Box.createVerticalStrut(15)) ;
			addControl(sidebar, "FLASH FRAME", frameBox) ;
			animateBox.setOpaque(false) ;
			animateBox.setForeground(TEXT) ;
			animateBox.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			sidebar.add(animateBox) ;
			sidebar.add(Box.createVerticalStrut(18)) ;
			sidebar.add(sectionLabel("SIMULATION")) ;
			sidebar.add(Box.createVerticalStrut(8)) ;
			fireButton = button("FIRE", ACCENT) ;
			fireButton.addMouseListener(new MouseAdapter() {
				public void mousePressed(MouseEvent event) {triggerHeld = true ; attemptFire() ;}
				public void mouseReleased(MouseEvent event) {triggerHeld = false ;}
				public void mouseExited(MouseEvent event) {
					if ((event.getModifiersEx() & MouseEvent.BUTTON1_DOWN_MASK) == 0) triggerHeld = false ;
				}
			}) ;
			sidebar.add(fireButton) ;
			sidebar.add(Box.createVerticalStrut(8)) ;
			reloadButton = button("RELOAD", PANEL_LIGHT) ;
			reloadButton.addActionListener(event -> startReload()) ;
			sidebar.add(reloadButton) ;
			sidebar.add(Box.createVerticalStrut(8)) ;
			actionProgress.setMaximumSize(new Dimension(Integer.MAX_VALUE, 12)) ;
			actionProgress.setStringPainted(false) ;
			sidebar.add(actionProgress) ;
			simulationStatus.setForeground(TEXT) ;
			simulationStatus.setFont(new Font("Dialog", Font.BOLD, 12)) ;
			simulationStatus.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			sidebar.add(Box.createVerticalStrut(5)) ;
			sidebar.add(simulationStatus) ;
			sidebar.add(Box.createVerticalStrut(18)) ;
			sidebar.add(sectionLabel("WEAPON DATA")) ;
			sidebar.add(Box.createVerticalStrut(8)) ;
			JButton configure = button("CONFIGURE WEAPON", PANEL_LIGHT) ;
			configure.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent event) {
					openSelectedWeaponEditor() ;
				}
			}) ;
			sidebar.add(configure) ;
			sidebar.add(Box.createVerticalStrut(8)) ;
			JButton addGun = button("ADD GUN", ACCENT) ;
			addGun.addActionListener(event -> addGun()) ;
			sidebar.add(addGun) ;
			sidebar.add(Box.createVerticalStrut(8)) ;
			JButton removeGun = button("REMOVE SELECTED GUN", new Color(138,45,45)) ;
			removeGun.setToolTipText("Remove the last appended custom gun; built-in and middle IDs cannot be removed") ;
			removeGun.addActionListener(event -> removeGun()) ;
			sidebar.add(removeGun) ;
			sidebar.add(Box.createVerticalStrut(8)) ;
			JButton reload = button("RELOAD FILES", PANEL_LIGHT) ;
			reload.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent event) {reloadResources() ;}
			}) ;
			sidebar.add(reload) ;
			sidebar.add(Box.createVerticalStrut(8)) ;
			JButton chooseRoot = button("SELECT GAME ROOT", PANEL_LIGHT) ;
			chooseRoot.addActionListener(event -> chooseRootFolder()) ;
			sidebar.add(chooseRoot) ;
			sidebar.add(Box.createVerticalStrut(12)) ;
			JLabel hint = new JLabel("Hover a preview and scroll to zoom") ;
			hint.setForeground(MUTED) ;
			hint.setFont(new Font("Dialog", Font.PLAIN, 11)) ;
			hint.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			sidebar.add(hint) ;
			JScrollPane scroll = new JScrollPane(sidebar, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER) ;
			scroll.setBorder(null) ;
			scroll.setPreferredSize(new Dimension(330, 700)) ;
			scroll.getViewport().setBackground(PANEL) ;
			return scroll ;
		}

		JComponent buildWeaponSelector() {
			JPanel panel = new JPanel(new BorderLayout(0,6)) ;
			panel.setOpaque(false) ;
			panel.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			panel.setMaximumSize(new Dimension(Integer.MAX_VALUE,180)) ;
			JLabel label = new JLabel("WEAPON SEARCH") ;
			label.setForeground(MUTED) ;
			label.setFont(new Font("Dialog",Font.BOLD,10)) ;
			panel.add(label,BorderLayout.NORTH) ;
			JPanel selection = new JPanel(new BorderLayout(0,6)) ;
			selection.setOpaque(false) ;
			weaponSearch.setToolTipText("Filter by weapon ID or name") ;
			selection.add(weaponSearch,BorderLayout.NORTH) ;
			weaponList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION) ;
			weaponList.setVisibleRowCount(6) ;
			weaponList.setFixedCellHeight(24) ;
			JScrollPane listScroll = new JScrollPane(weaponList,JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,JScrollPane.HORIZONTAL_SCROLLBAR_NEVER) ;
			selection.add(listScroll,BorderLayout.CENTER) ;
			panel.add(selection,BorderLayout.CENTER) ;
			return panel ;
		}

		JLabel sectionLabel(String text) {
			JLabel label = new JLabel(text) ;
			label.setForeground(ACCENT) ;
			label.setFont(new Font("Dialog", Font.BOLD, 11)) ;
			label.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			return label ;
		}

		void addControl(JPanel panel, String labelText, JComponent control) {
			JLabel label = new JLabel(labelText) ;
			label.setForeground(MUTED) ;
			label.setFont(new Font("Dialog", Font.BOLD, 10)) ;
			label.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			control.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			int controlHeight = control instanceof JSlider ? 56 : 28 ;
			control.setMinimumSize(new Dimension(210, controlHeight)) ;
			control.setPreferredSize(new Dimension(244, controlHeight)) ;
			control.setMaximumSize(new Dimension(Integer.MAX_VALUE, controlHeight)) ;
			panel.add(label) ;
			panel.add(Box.createVerticalStrut(4)) ;
			panel.add(control) ;
			panel.add(Box.createVerticalStrut(15)) ;
		}

		void configureSlider(JSlider slider, int major, int minor) {
			slider.setOpaque(false) ;
			slider.setForeground(TEXT) ;
			slider.setBorder(new EmptyBorder(0, 10, 0, 10)) ;
			slider.setMajorTickSpacing(major) ;
			slider.setMinorTickSpacing(minor) ;
			slider.setPaintTicks(true) ;
			slider.setPaintLabels(true) ;
		}

		JButton button(String text, Color color) {
			JButton button = new JButton(text) ;
			button.setAlignmentX(Component.LEFT_ALIGNMENT) ;
			button.setMinimumSize(new Dimension(180, 34)) ;
			button.setPreferredSize(new Dimension(214, 34)) ;
			button.setMaximumSize(new Dimension(Integer.MAX_VALUE, 34)) ;
			button.setContentAreaFilled(false) ;
			button.setOpaque(true) ;
			button.setBackground(color) ;
			button.setForeground(TEXT) ;
			button.setFocusPainted(false) ;
			button.setBorder(BorderFactory.createLineBorder(color.brighter())) ;
			return button ;
		}

		void installListeners() {
			ActionListener action = new ActionListener() {
				public void actionPerformed(ActionEvent event) {selectionChanged() ;}
			} ;
			previewBox.addActionListener(action) ;
			weaponList.addListSelectionListener(event -> {if (!event.getValueIsAdjusting()) selectionChanged() ;}) ;
			weaponSearch.getDocument().addDocumentListener(new javax.swing.event.DocumentListener() {
				public void insertUpdate(javax.swing.event.DocumentEvent event) {filterWeapons() ;}
				public void removeUpdate(javax.swing.event.DocumentEvent event) {filterWeapons() ;}
				public void changedUpdate(javax.swing.event.DocumentEvent event) {filterWeapons() ;}
			}) ;
			teamBox.addActionListener(action) ;
			modelBox.addActionListener(action) ;
			frameBox.addActionListener(action) ;
			animateBox.addActionListener(action) ;
			lightPreviewBox.addActionListener(action) ;
			angleSlider.addChangeListener(event -> selectionChanged()) ;
		}

		void reloadResources() {
			WeaponInfo selected = selectedWeapon() ;
			int selectedId = selected == null ? 0 : selected.id ;
			reloadResources(selectedId) ;
		}

		void reloadResources(int selectedId) {
			try {
				model.load() ;
				filterWeapons(selectedId) ;
				resetSimulation() ;
				selectionChanged() ;
			} catch (Exception exception) {
				status.setText("File error: " + exception.getMessage()) ;
				JOptionPane.showMessageDialog(this, exception.getMessage(), "File error", JOptionPane.ERROR_MESSAGE) ;
			}
		}

		void filterWeapons() {
			WeaponInfo selected = selectedWeapon() ;
			filterWeapons(selected == null ? -1 : selected.id) ;
		}

		void filterWeapons(int selectedId) {
			String query = weaponSearch.getText().trim().toLowerCase() ;
			weaponListModel.clear() ;
			int selection = -1 ;
			for (WeaponInfo weapon : model.weapons) {
				String id = Integer.toString(weapon.id) ;
				if (query.length() > 0 && !id.contains(query) && !weapon.name.toLowerCase().contains(query)) continue ;
				if (weapon.id == selectedId) selection = weaponListModel.size() ;
				weaponListModel.addElement(weapon) ;
			}
			if (selection < 0 && !weaponListModel.isEmpty()) selection = 0 ;
			if (selection >= 0) weaponList.setSelectedIndex(selection) ;
		}

		void addGun() {
			int id = model.weapons.size() ;
			if (id >= 128) {
				JOptionPane.showMessageDialog(this, "The maximum of 128 weapons has been reached.", "Cannot add gun", JOptionPane.ERROR_MESSAGE) ;
				return ;
			}
			new WeaponEditorDialog(this, WeaponInfo.createDefault(id), true).setVisible(true) ;
		}

		void removeGun() {
			WeaponInfo weapon = selectedWeapon() ;
			if (weapon == null) return ;
			int lastId = model.weapons.isEmpty() ? -1 : model.weapons.get(model.weapons.size()-1).id ;
			if (weapon.id < FIRST_CUSTOM_GUN_ID) {
				JOptionPane.showMessageDialog(this,"Weapon IDs 0-66 are built into the game and cannot be removed.",
				                              "Cannot remove gun",JOptionPane.ERROR_MESSAGE) ;
				return ;
			}
			if (weapon.id != lastId) {
				JOptionPane.showMessageDialog(this,"Only the highest weapon ID can be removed. Remove ID " + lastId + " first to keep gun IDs contiguous.",
				                              "Cannot remove gun",JOptionPane.ERROR_MESSAGE) ;
				return ;
			}
			int answer = JOptionPane.showConfirmDialog(this,"Remove weapon " + weapon.id + " (" + weapon.name + ")?\nIts tracer and laser settings will also be removed. Sprite cells remain available for ID reuse.",
			                                               "Remove gun",JOptionPane.YES_NO_OPTION,JOptionPane.WARNING_MESSAGE) ;
			if (answer != JOptionPane.YES_OPTION) return ;
			try {
				model.removeLastWeapon() ;
				reloadResources(Math.max(0,weapon.id-1)) ;
				JOptionPane.showMessageDialog(this,"Removed weapon " + weapon.id + " (" + weapon.name + ").",
				                              "Gun removed",JOptionPane.INFORMATION_MESSAGE) ;
			} catch (IOException exception) {
				JOptionPane.showMessageDialog(this,exception.getMessage(),"Remove failed",JOptionPane.ERROR_MESSAGE) ;
			}
		}

		void chooseRootFolder() {
			JFileChooser chooser = new JFileChooser(model.requestedRoot == null ? null : model.requestedRoot) ;
			chooser.setDialogTitle("Select the CSPSP folder containing bin") ;
			chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY) ;
			chooser.setAcceptAllFileFilterUsed(false) ;
			if (chooser.showOpenDialog(this) != JFileChooser.APPROVE_OPTION) return ;
			File previous = model.requestedRoot ;
			model.setRoot(chooser.getSelectedFile()) ;
			try {
				model.resolveContentRoot(model.requestedRoot) ;
				reloadResources() ;
			} catch (IOException exception) {
				model.requestedRoot = previous ;
				if (previous == null) ResourceModel.PREFERENCES.remove("gameRoot") ;
				else ResourceModel.PREFERENCES.put("gameRoot", previous.getAbsolutePath()) ;
				JOptionPane.showMessageDialog(this, exception.getMessage(), "Invalid game folder", JOptionPane.ERROR_MESSAGE) ;
			}
		}

		void selectionChanged() {
			WeaponInfo weapon = selectedWeapon() ;
			if (weapon != null) activeWeapon = weapon ;
			if (weapon != null && weapon.id != simulatedWeaponId) resetSimulation() ;
			boolean guns = previewBox.getSelectedIndex() == 0 ;
			weaponSearch.setEnabled(guns) ;
			weaponList.setEnabled(guns) ;
			frameBox.setEnabled(guns) ;
			animateBox.setEnabled(guns) ;
			if (fireButton != null) fireButton.setEnabled(guns) ;
			if (reloadButton != null) reloadButton.setEnabled(guns) ;
			teamBox.setEnabled(!guns) ;
			modelBox.setEnabled(!guns) ;
			angleSlider.setEnabled(true) ;
			if (guns && weapon != null) {
				boolean fallback = !model.flashes.containsKey(Integer.valueOf(weapon.flashStyle)) ;
				TracerConfig tracer = model.tracer(weapon.id) ;
				String flashName = weapon.flashStyle == 0 ? "muzzleflash.png" : "muzzleflash" + weapon.flashStyle + ".png" ;
				status.setText("ID " + weapon.id + "  |  " + weapon.name + "  |  " + weapon.weaponType()
				               + " / " + weapon.bulletType() + "  |  speed " + weapon.bulletSpeed
				               + "  |  tracer " + tracer.style
				               + "  |  flash " + weapon.flashStyle + " (" + flashName
				               + (fallback ? ", fallback to style 0" : "") + ")") ;
			} else if (!guns && model.directory != null) {
				status.setText(teamBox.getSelectedItem() + "  |  " + modelBox.getSelectedItem() + "  |  players.png") ;
			}
			canvas.repaint() ;
		}

		void openSelectedWeaponEditor() {
			WeaponInfo weapon = selectedWeapon() ;
			if (weapon == null) weapon = activeWeapon ;
			if (weapon == null) {
				JOptionPane.showMessageDialog(this,"No weapon is selected.","Cannot configure weapon",JOptionPane.ERROR_MESSAGE) ;
				return ;
			}
			try {
				WeaponEditorDialog dialog = new WeaponEditorDialog(this,weapon) ;
				dialog.setLocationRelativeTo(this) ;
				dialog.setVisible(true) ;
			}
			catch (RuntimeException exception) {
				String message = exception.getMessage() == null ? exception.getClass().getSimpleName() : exception.getMessage() ;
				JOptionPane.showMessageDialog(this,message,"Cannot configure weapon",JOptionPane.ERROR_MESSAGE) ;
			}
		}

		void resetSimulation() {
			WeaponInfo weapon = selectedWeapon() ;
			if (weapon == null) return ;
			simulatedWeaponId = weapon.id ;
			magazineAmmo = Math.max(0, weapon.clip) ;
			reserveAmmo = Math.max(0, weapon.clip * Math.max(0, weapon.numClips - 1)) ;
			shotsFired = 0 ; nextShotAt = 0 ; reloadStartedAt = 0 ; reloadUntil = 0 ;
			triggerHeld = false ; reloading = false ;
			tracerShots.clear() ;
			actionProgress.setValue(1000) ;
			updateSimulationStatus("READY") ;
		}

		void attemptFire() {
			WeaponInfo weapon = selectedWeapon() ;
			if (weapon == null || previewBox.getSelectedIndex() != 0 || reloading) return ;
			long now = System.currentTimeMillis() ;
			if (now < nextShotAt) return ;
			boolean usesAmmo = weapon.type != 2 && weapon.clip > 0 ;
			if (usesAmmo && magazineAmmo <= 0) {
				updateSimulationStatus("EMPTY") ;
				triggerHeld = false ;
				return ;
			}
			if (usesAmmo) magazineAmmo-- ;
			shotsFired++ ;
			nextShotAt = now + Math.max(0, weapon.delay) ;
			animatedFrame = (int)(Math.random() * 3) ;
			flashUntil = now + Math.max(80, Math.min(180, weapon.delay)) ;
			addTracerShots(weapon, now) ;
			updateSimulationStatus("FIRING") ;
			canvas.repaint() ;
			if (weapon.fireMode == 0) triggerHeld = false ;
		}

		void addTracerShots(WeaponInfo weapon, long now) {
			if (weapon.type == 2 || weapon.type == 3 || (weapon.projectileType != 1 && "none".equals(model.tracer(weapon.id).style))) return ;
			double aim = Math.toRadians(angleSlider.getValue()) ;
			if (weapon.pellets <= 1) {
				tracerShots.add(new TracerShot(weapon.id, now, aim)) ;
				return ;
			}
			double angle = aim - weapon.spread * 0.5 ;
			double step = weapon.spread / Math.max(1, weapon.pellets - 1) ;
			int randomSteps = weapon.pellets == 4 ? 10 : 11 ;
			for (int index = 0 ; index < weapon.pellets ; index++) {
				angle += ((int)(Math.random() * randomSteps)) / 100.0 - 0.05 ;
				tracerShots.add(new TracerShot(weapon.id, now, angle)) ;
				angle += step ;
			}
		}

		void startReload() {
			WeaponInfo weapon = selectedWeapon() ;
			if (weapon == null || reloading || magazineAmmo >= weapon.clip || reserveAmmo <= 0) return ;
			long now = System.currentTimeMillis() ;
			reloading = true ; triggerHeld = false ; reloadStartedAt = now ; reloadUntil = now + Math.max(0, weapon.reloadDelay) ;
			updateSimulationStatus("RELOADING") ;
		}

		void updateSimulation() {
			WeaponInfo weapon = selectedWeapon() ;
			if (weapon == null) return ;
			long now = System.currentTimeMillis() ;
			for (int index = tracerShots.size() - 1 ; index >= 0 ; index--) {
				if (now - tracerShots.get(index).startedAt > 1500) tracerShots.remove(index) ;
			}
			if (!tracerShots.isEmpty()) canvas.repaint() ;
			if (reloading) {
				long duration = Math.max(1, reloadUntil - reloadStartedAt) ;
				actionProgress.setValue((int)Math.min(1000, (now - reloadStartedAt) * 1000 / duration)) ;
				if (now >= reloadUntil) {
					int transferred = Math.min(Math.max(0, weapon.clip - magazineAmmo), reserveAmmo) ;
					magazineAmmo += transferred ; reserveAmmo -= transferred ; reloading = false ;
					updateSimulationStatus("READY") ;
				}
			} else {
				long duration = Math.max(1, weapon.delay) ;
				actionProgress.setValue((int)Math.min(1000, Math.max(0, now - (nextShotAt - duration)) * 1000 / duration)) ;
				if (triggerHeld && weapon.fireMode == 1) attemptFire() ;
				else if (now >= nextShotAt && simulationStatus.getText().startsWith("FIRING")) updateSimulationStatus("READY") ;
			}
		}

		void updateSimulationStatus(String state) {
			WeaponInfo weapon = selectedWeapon() ;
			if (weapon == null) return ;
			String ammo = weapon.type == 2 ? "unlimited" : magazineAmmo + " / " + reserveAmmo ;
			simulationStatus.setText(state + "   " + ammo + "   shots " + shotsFired) ;
		}

		WeaponInfo selectedWeapon() {return weaponList.getSelectedValue() ;}
		int flashFrame() {return animateBox.isSelected() ? animatedFrame : frameBox.getSelectedIndex() ;}
		boolean flashVisible() {return animateBox.isSelected() || System.currentTimeMillis() < flashUntil ;}
	}

	static class PreviewCanvas extends JPanel implements MouseWheelListener {
		final PreviewFrame frame ;
		final Rectangle[] gunPanels = {new Rectangle(), new Rectangle(), new Rectangle(), new Rectangle()} ;
		final Rectangle playerPanel = new Rectangle() ;
		final int[] gunZoom = {5, 5, 5, 4} ;
		int playerZoom = 5 ;

		PreviewCanvas(PreviewFrame frame) {
			this.frame = frame ;
			setBackground(BACKGROUND) ;
			setPreferredSize(new Dimension(820, 620)) ;
			setToolTipText("Scroll over a preview panel to zoom it") ;
			addMouseWheelListener(this) ;
		}

		public void mouseWheelMoved(MouseWheelEvent event) {
			int direction = event.getWheelRotation() < 0 ? 1 : -1 ;
			if (frame.previewBox.getSelectedIndex() == 0) {
				for (int index = 0 ; index < gunPanels.length ; index++) {
					if (gunPanels[index].contains(event.getPoint())) {
						gunZoom[index] = clampZoom(gunZoom[index] + direction) ;
						repaint() ;
						return ;
					}
				}
			} else if (playerPanel.contains(event.getPoint())) {
				playerZoom = clampZoom(playerZoom + direction) ;
				repaint() ;
			}
		}

		int clampZoom(int zoom) {return Math.max(1, Math.min(12, zoom)) ;}

		protected void paintComponent(Graphics graphics) {
			super.paintComponent(graphics) ;
			Graphics2D g = (Graphics2D)graphics.create() ;
			try {
				g.setColor(previewBackground()) ;
				g.fillRect(0, 0, getWidth(), getHeight()) ;
				g.setRenderingHint(RenderingHints.KEY_INTERPOLATION, RenderingHints.VALUE_INTERPOLATION_NEAREST_NEIGHBOR) ;
				g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON) ;
				drawGrid(g) ;
				if (frame.previewBox.getSelectedIndex() == 0) drawGunPreview(g) ;
				else drawPlayerPreview(g) ;
			} finally {g.dispose() ;}
		}

		void layoutGunPanels() {
			int sideWidth = Math.max(210, Math.min(260, getWidth() / 3)) ;
			int mainWidth = getWidth() - sideWidth ;
			int third = (getHeight() - 60) / 3 ;
			gunPanels[0].setBounds(22, 22, mainWidth - 34, getHeight() - 44) ;
			gunPanels[1].setBounds(mainWidth + 4, 22, sideWidth - 26, third) ;
			gunPanels[2].setBounds(mainWidth + 4, 36 + third, sideWidth - 26, third) ;
			gunPanels[3].setBounds(mainWidth + 4, 50 + 2 * third, sideWidth - 26, third - 6) ;
		}

		void drawGunPreview(Graphics2D g) {
			WeaponInfo weapon = frame.selectedWeapon() ;
			if (weapon == null) {drawCentered(g, "No weapon data loaded", getWidth() / 2, getHeight() / 2, MUTED) ; return ;}
			layoutGunPanels() ;
			drawSection(g, gunPanels[0], "WEAPON + MUZZLE  |  " + gunZoom[0] + "x") ;
			drawSection(g, gunPanels[1], "GROUND  |  " + gunZoom[1] + "x") ;
			drawSection(g, gunPanels[2], "WEAPON SPRITE  |  " + gunZoom[2] + "x") ;
			drawSection(g, gunPanels[3], "MUZZLE FLASH  |  " + gunZoom[3] + "x") ;
			double centerX = gunPanels[0].getCenterX() ;
			double centerY = gunPanels[0].getCenterY() + 15 ;
			double aimAngle = Math.toRadians(frame.angleSlider.getValue()) ;
			double spriteAngle = aimAngle - Math.PI / 2.0 ;
			double directionX = Math.cos(aimAngle) ;
			double directionY = Math.sin(aimAngle) ;
			double[] gunAnchor = idleGunAnchor(weapon) ;
			double gunX = centerX + (directionX*gunAnchor[0]-directionY*gunAnchor[1])*gunZoom[0] ;
			double gunY = centerY + (directionY*gunAnchor[0]+directionX*gunAnchor[1])*gunZoom[0] ;
			drawWeaponLaser(g, weapon, centerX, centerY, aimAngle, gunZoom[0]) ;
			drawWeaponCell(g, frame.model.handPage(weapon.id), weapon.id, gunX, gunY, 16, 8, spriteAngle, gunZoom[0]) ;
			if (hasMuzzleFlash(weapon) && frame.flashVisible()) {
				BufferedImage flash = frame.model.flash(weapon.flashStyle) ;
				float[] muzzle = frame.model.muzzlePosition(weapon.id) ;
				double muzzleX = gunX+(directionX*muzzle[0]-directionY*muzzle[1])*gunZoom[0] ;
				double muzzleY = gunY+(directionY*muzzle[0]+directionX*muzzle[1])*gunZoom[0] ;
				drawQuad(g, flash, frame.flashFrame() * 32, 0, 32, 32, muzzleX, muzzleY, 16, -16, spriteAngle, gunZoom[0], false) ;
			}
			drawLiveTracers(g, weapon, centerX, centerY) ;
			String metadata = weapon.weaponType() + " / " + weapon.bulletType() + " / SPEED " + weapon.bulletSpeed ;
			drawCentered(g, metadata, (int)centerX, gunPanels[0].y + 48, labelColor()) ;
			drawWeaponCell(g, frame.model.groundPage(weapon.id), weapon.id, gunPanels[1].getCenterX(), gunPanels[1].getCenterY() + 8, 16, 16, 0, gunZoom[1]) ;
			drawWeaponCell(g, frame.model.handPage(weapon.id), weapon.id, gunPanels[2].getCenterX(), gunPanels[2].getCenterY() + 8, 16, 16, 0, gunZoom[2]) ;
			drawFlash(g, weapon, (int)gunPanels[3].getCenterX(), (int)gunPanels[3].getCenterY() + 8, gunZoom[3]) ;
		}

		double[] idleGunAnchor(WeaponInfo weapon) {
			double[] pose = frame.model.animationFrame(frame.model.idleAnimationName(weapon),0).angles ;
			double body = Math.toRadians(pose[0]) ;
			double rightArm = Math.toRadians(pose[1]) ;
			double rightHand = Math.toRadians(pose[2]) ;
			return new double[] {
				-5.0-10.0*Math.sin(body)+8.0*Math.cos(rightArm)+10.0*Math.cos(rightHand)+weapon.spriteOffsetX,
				10.0*Math.cos(body)+8.0*Math.sin(rightArm)+10.0*Math.sin(rightHand)+weapon.spriteOffsetY
			} ;
		}

		double[] drawPosedWeapon(Graphics2D g, WeaponInfo weapon, String poseName, double offsetX, double offsetY, double originX, double originY, double zoom) {
			if (frame.model.players == null) return new double[] {originX,originY,0.0} ;
			double[] pose = frame.model.animationFrame(poseName,0).angles ;
			return drawPosedWeapon(g,weapon,pose,offsetX,offsetY,originX,originY,zoom) ;
		}

		double[] drawPosedWeapon(Graphics2D g, WeaponInfo weapon, double[] pose, double offsetX, double offsetY, double originX, double originY, double zoom) {
			return drawPosedWeapon(g,weapon,pose,offsetX,offsetY,originX,originY,zoom,true) ;
		}

		double[] drawPosedWeapon(Graphics2D g, WeaponInfo weapon, double[] pose, double offsetX, double offsetY, double originX, double originY, double zoom, boolean drawLegs) {
			if (frame.model.players == null) return new double[] {originX,originY,0.0} ;
			double rotation = -Math.PI/2.0, chainRotation = 0.0, body = Math.toRadians(pose[0]) ;
			double rightArm = Math.toRadians(pose[1]), rightHand = Math.toRadians(pose[2]) ;
			double leftArm = Math.toRadians(pose[3]), leftHand = Math.toRadians(pose[4]) ;
			int atlasTeam = frame.teamBox.getSelectedIndex() == 0 ? 1 : 0 ;
			int playerOffset = (frame.modelBox.getSelectedIndex()+atlasTeam*4)*32 ;
			BufferedImage players = frame.model.players ;
			if (drawLegs) drawQuad(g,players,playerOffset,32,32,32,originX,originY,16,16,rotation,zoom,false) ;
			double centerX=originX-5*Math.cos(chainRotation)*zoom, centerY=originY-5*Math.sin(chainRotation)*zoom ;
			drawQuad(g,players,playerOffset,0,32,16,centerX,centerY,16,8,rotation+body,zoom,false) ;
			double bodyDx=10*Math.cos(rotation+body)*zoom, bodyDy=10*Math.sin(rotation+body)*zoom ;
			double leftX=centerX+bodyDx, leftY=centerY+bodyDy ;
			drawQuad(g,players,playerOffset+8,16,8,16,leftX,leftY,4,4,rotation+leftArm,zoom,true) ;
			leftX+=8*Math.cos(chainRotation+leftArm)*zoom ; leftY+=8*Math.sin(chainRotation+leftArm)*zoom ;
			drawQuad(g,players,playerOffset,16,8,16,leftX,leftY,4,3,rotation+leftHand,zoom,true) ;
			double rightX=centerX-bodyDx, rightY=centerY-bodyDy ;
			drawQuad(g,players,playerOffset+8,16,8,16,rightX,rightY,4,4,rotation+rightArm,zoom,false) ;
			rightX+=8*Math.cos(chainRotation+rightArm)*zoom ; rightY+=8*Math.sin(chainRotation+rightArm)*zoom ;
			drawQuad(g,players,playerOffset,16,8,16,rightX,rightY,4,3,rotation+rightHand,zoom,false) ;
			double gunX=rightX+10*Math.cos(chainRotation+rightHand)*zoom+offsetX*zoom ;
			double gunY=rightY+10*Math.sin(chainRotation+rightHand)*zoom+offsetY*zoom ;
			drawWeaponCell(g,frame.model.handPage(weapon.id),weapon.id,gunX,gunY,16,8,rotation+Math.toRadians(pose[5]),zoom) ;
			drawQuad(g,players,playerOffset+16,16,16,16,centerX,centerY,8,7,rotation,zoom,false) ;
			return new double[] {gunX,gunY,Math.toRadians(pose[5])} ;
		}

		void drawWeaponLaser(Graphics2D g, WeaponInfo weapon, double centerX, double centerY, double aimAngle, double zoom) {
			LaserConfig laser = frame.model.laser(weapon.id) ;
			if (!laser.enabled) return ;
			double directionX = Math.cos(aimAngle) ;
			double directionY = Math.sin(aimAngle) ;
			double startX = centerX + (directionX * laser.offsetX - directionY * laser.offsetY) * zoom ;
			double startY = centerY + (directionY * laser.offsetX + directionX * laser.offsetY) * zoom ;
			double endX = startX + directionX * laser.range * zoom ;
			double endY = startY + directionY * laser.range * zoom ;
			int segments = laser.falloffEnabled && laser.falloff > 0.0f ? 12 : 1 ;
			Shape oldClip = g.getClip() ;
			g.clip(gunPanels[0]) ;
			for (int index = 0 ; index < segments ; index++) {
				double from = index / (double)segments ;
				double to = (index + 1) / (double)segments ;
				double intensity = segments == 1 ? 1.0 : Math.pow(Math.max(0.0, 1.0 - (from + to) * 0.5), laser.falloff) ;
				Color color = new Color(laser.red, laser.green, laser.blue, (int)(laser.alpha * intensity)) ;
				drawTracerLine(g, startX + (endX - startX) * from, startY + (endY - startY) * from,
				               startX + (endX - startX) * to, startY + (endY - startY) * to,
				               laser.width * (float)zoom, color) ;
			}
			if (laser.endDot) {
				double intensity = laser.falloffEnabled && laser.falloff > 0.0f ? 0.15 : 1.0 ;
				drawTracerPoint(g, endX, endY, laser.endDotScale * (float)zoom,
				                new Color(laser.red, laser.green, laser.blue, (int)(laser.alpha * intensity))) ;
			}
			g.setClip(oldClip) ;
		}

		void drawPlayerPreview(Graphics2D g) {
			if (frame.model.players == null) {drawCentered(g, "No player data loaded", getWidth() / 2, getHeight() / 2, MUTED) ; return ;}
			playerPanel.setBounds(22, 22, getWidth() - 44, getHeight() - 44) ;
			drawSection(g, playerPanel, "STATIC PLAYER MODEL  |  " + playerZoom + "x") ;
			drawPlayer(g, playerPanel.getCenterX(), playerPanel.getCenterY() + 16, playerZoom) ;
		}

		void drawGrid(Graphics2D g) {
			g.setColor(frame.lightPreviewBox.isSelected() ? new Color(215, 218, 220) : new Color(25, 28, 32)) ;
			for (int x = 0 ; x < getWidth() ; x += 24) g.drawLine(x, 0, x, getHeight()) ;
			for (int y = 0 ; y < getHeight() ; y += 24) g.drawLine(0, y, getWidth(), y) ;
		}

		void drawSection(Graphics2D g, Rectangle bounds, String label) {
			boolean light = frame.lightPreviewBox.isSelected() ;
			g.setColor(light ? new Color(248, 249, 247) : PANEL) ;
			g.fillRoundRect(bounds.x, bounds.y, bounds.width, bounds.height, 8, 8) ;
			g.setColor(light ? new Color(181, 187, 191) : new Color(55, 61, 68)) ;
			g.drawRoundRect(bounds.x, bounds.y, bounds.width, bounds.height, 8, 8) ;
			g.setFont(new Font("Dialog", Font.BOLD, 10)) ;
			g.setColor(labelColor()) ;
			g.drawString(label, bounds.x + 12, bounds.y + 18) ;
		}

		Color previewBackground() {return frame.lightPreviewBox.isSelected() ? new Color(232, 235, 236) : BACKGROUND ;}
		Color labelColor() {return frame.lightPreviewBox.isSelected() ? new Color(70, 76, 82) : MUTED ;}

		void drawPlayer(Graphics2D g, double originX, double originY, double zoom) {
			double facing = Math.toRadians(frame.angleSlider.getValue()) ;
			double rotation = facing - Math.PI / 2.0 ;
			int atlasTeam = frame.teamBox.getSelectedIndex() == 0 ? 1 : 0 ;
			int playerOffset = (frame.modelBox.getSelectedIndex() + atlasTeam * 4) * 32 ;
			BufferedImage players = frame.model.players ;
			drawQuad(g, players, playerOffset, 32, 32, 32, originX, originY, 16, 16, rotation, zoom, false) ;
			double centerX = originX - 5 * Math.cos(facing) * zoom ;
			double centerY = originY - 5 * Math.sin(facing) * zoom ;
			double bodyAngle = 0.4 ;
			double dx = 10 * Math.cos(rotation + bodyAngle) * zoom ;
			double dy = 10 * Math.sin(rotation + bodyAngle) * zoom ;
			drawQuad(g, players, playerOffset, 0, 32, 16, centerX, centerY, 16, 8, rotation + bodyAngle, zoom, false) ;
			double leftX = centerX + dx ;
			double leftY = centerY + dy ;
			drawQuad(g, players, playerOffset + 8, 16, 8, 16, leftX, leftY, 4, 4, rotation, zoom, true) ;
			leftX += 8 * Math.cos(facing) * zoom ;
			leftY += 8 * Math.sin(facing) * zoom ;
			drawQuad(g, players, playerOffset, 16, 8, 16, leftX, leftY, 4, 3, rotation, zoom, true) ;
			double rightX = centerX - dx ;
			double rightY = centerY - dy ;
			drawQuad(g, players, playerOffset + 8, 16, 8, 16, rightX, rightY, 4, 4, rotation, zoom, false) ;
			rightX += 8 * Math.cos(facing) * zoom ;
			rightY += 8 * Math.sin(facing) * zoom ;
			drawQuad(g, players, playerOffset, 16, 8, 16, rightX, rightY, 4, 3, rotation, zoom, false) ;
			drawQuad(g, players, playerOffset + 16, 16, 16, 16, centerX, centerY, 8, 7, rotation, zoom, false) ;
		}

		void drawLiveTracers(Graphics2D g, WeaponInfo weapon, double centerX, double centerY) {
			TracerConfig tracer = frame.model.tracer(weapon.id) ;
			if (weapon.projectileType != 1 && "none".equals(tracer.style)) return ;
			long now = System.currentTimeMillis() ;
			Shape oldClip = g.getClip() ;
			g.clip(gunPanels[0]) ;
			for (TracerShot shot : frame.tracerShots) {
				if (shot.weaponId != weapon.id) continue ;
				double elapsed = now - shot.startedAt ;
				double travel = 0.3 * weapon.bulletSpeed * elapsed * gunZoom[0] ;
				boolean shoulder = frame.model.projectileLaunchOrigin(weapon) == 1 ;
				double muzzleForward = shoulder ? 28.0 : 24.0 ;
				double muzzleSide = shoulder ? 8.0 : 0.0 ;
				double muzzleX = centerX + (Math.cos(shot.angle)*muzzleForward-Math.sin(shot.angle)*muzzleSide)*gunZoom[0] ;
				double muzzleY = centerY + (Math.sin(shot.angle)*muzzleForward+Math.cos(shot.angle)*muzzleSide)*gunZoom[0] ;
				double tipX = muzzleX + Math.cos(shot.angle)*travel ;
				double tipY = muzzleY + Math.sin(shot.angle)*travel ;
				if (!"none".equals(tracer.style)) drawTracer(g,tracer,tipX,tipY,shot.angle,gunZoom[0],weapon,travel) ;
				if (weapon.projectileType == 1 && frame.model.projectileAtlas != null) {
					int style=weapon.projectileStyle, column=style%4, row=style/4 ;
					drawQuad(g,frame.model.projectileAtlas,column*32,row*32,32,32,tipX,tipY,16,16,shot.angle,gunZoom[0],false) ;
				}
			}
			g.setClip(oldClip) ;
		}

		double tracerLength(TracerConfig tracer, WeaponInfo weapon) {
			return tracer.length > 0.0f ? tracer.length : Math.max(18.0, weapon.bulletSpeed * 21.0) ;
		}

		void drawTracerCentered(Graphics2D g, TracerConfig tracer, WeaponInfo weapon, Rectangle panel,
		                        double angle, int zoom) {
			if ("none".equals(tracer.style)) {
				drawCentered(g, "none", (int)panel.getCenterX(), (int)panel.getCenterY() + 8, labelColor()) ;
				return ;
			}
			double length = tracerLength(tracer, weapon) * zoom ;
			double tipX = panel.getCenterX() + Math.cos(angle) * length * 0.5 ;
			double tipY = panel.getCenterY() + 8 + Math.sin(angle) * length * 0.5 ;
			Shape oldClip = g.getClip() ;
			g.clip(panel) ;
			drawTracer(g, tracer, tipX, tipY, angle, zoom, weapon, Double.POSITIVE_INFINITY) ;
			g.setClip(oldClip) ;
		}

		void drawTracer(Graphics2D g, TracerConfig tracer, double tipX, double tipY,
		                double angle, double zoom, WeaponInfo weapon, double maximumLength) {
			if ("none".equals(tracer.style)) return ;
			double length = Math.min(tracerLength(tracer, weapon) * zoom, maximumLength) ;
			double directionX = Math.cos(angle) ;
			double directionY = Math.sin(angle) ;
			double sideX = -directionY ;
			double sideY = directionX ;
			double startX = tipX - directionX * length ;
			double startY = tipY - directionY * length ;
			float width = Math.max(1.0f, tracer.width * (float)zoom) ;
			Color color = tracer.color() ;
			Color dim = tracerColor(tracer, Math.max(1, tracer.alpha / 3), 0.4) ;
			String style = tracer.style ;

			if ("dot".equals(style)) {
				drawTracerPoint(g, tipX, tipY, width, color) ;
				return ;
			}
			if ("dashed".equals(style)) {
				for (int index = 0 ; index < 3 ; index++) {
					double near = index * 0.3 ;
					double far = near + 0.15 ;
					drawTracerLine(g, tipX - directionX * length * far, tipY - directionY * length * far,
					               tipX - directionX * length * near, tipY - directionY * length * near, width, color) ;
				}
			} else if ("laser".equals(style) || "beam".equals(style)) {
				float glow = "beam".equals(style) ? 5.0f : 3.0f ;
				drawTracerLine(g, startX, startY, tipX, tipY, width * glow, dim) ;
				drawTracerLine(g, startX, startY, tipX, tipY, width, color) ;
				if ("beam".equals(style)) drawTracerLine(g, startX, startY, tipX, tipY, width * 0.5f, Color.WHITE) ;
			} else if ("plasma".equals(style) || "comet".equals(style)) {
				double scale = "plasma".equals(style) ? 0.6 : 0.45 ;
				drawTracerLine(g, tipX - directionX * length * scale, tipY - directionY * length * scale,
				               tipX, tipY, width * 2.0f, color) ;
				drawTracerPoint(g, tipX, tipY, width * 2.0f, color) ;
				return ;
			} else if ("bolt".equals(style)) {
				double head = (5.0 + tracer.width * 2.0) * zoom ;
				double baseX = tipX - directionX * head ;
				double baseY = tipY - directionY * head ;
				drawTracerLine(g, startX, startY, tipX, tipY, width, color) ;
				drawTracerLine(g, tipX, tipY, baseX + sideX * head * 0.5, baseY + sideY * head * 0.5, width, color) ;
				drawTracerLine(g, tipX, tipY, baseX - sideX * head * 0.5, baseY - sideY * head * 0.5, width, color) ;
				return ;
			} else if ("pulse".equals(style)) {
				for (int index = 0 ; index < 5 ; index++) {
					double scale = index * 0.2 ;
					drawTracerPoint(g, tipX - directionX * length * scale, tipY - directionY * length * scale,
					                width, tracerColor(tracer, tracer.alpha * (5 - index) / 5, 1.0)) ;
				}
				return ;
			} else if ("rail".equals(style) || "twin".equals(style)) {
				double spacing = tracer.width * zoom * ("rail".equals(style) ? 2.0 : 1.5) ;
				drawTracerLine(g, startX + sideX * spacing, startY + sideY * spacing,
				               tipX + sideX * spacing, tipY + sideY * spacing, width, color) ;
				drawTracerLine(g, startX - sideX * spacing, startY - sideY * spacing,
				               tipX - sideX * spacing, tipY - sideY * spacing, width, color) ;
				if ("rail".equals(style)) drawTracerLine(g, startX, startY, tipX, tipY, width * 0.5f, Color.WHITE) ;
			} else if ("spark".equals(style) || "flare".equals(style)) {
				double trail = "flare".equals(style) ? 0.4 : 0.25 ;
				drawTracerLine(g, tipX - directionX * length * trail, tipY - directionY * length * trail,
				               tipX, tipY, width, color) ;
				double radius = (("flare".equals(style) ? 4.0 : 3.0) + tracer.width * 3.0) * zoom ;
				drawTracerLine(g, tipX - radius, tipY, tipX + radius, tipY, width, color) ;
				drawTracerLine(g, tipX, tipY - radius, tipX, tipY + radius, width, color) ;
				if ("flare".equals(style)) {
					double diagonal = radius * 0.7 ;
					drawTracerLine(g, tipX - diagonal, tipY - diagonal, tipX + diagonal, tipY + diagonal, width, color) ;
					drawTracerLine(g, tipX - diagonal, tipY + diagonal, tipX + diagonal, tipY - diagonal, width, color) ;
				}
			} else if ("needle".equals(style)) {
				drawTracerLine(g, tipX - directionX * length * 0.35, tipY - directionY * length * 0.35,
				               tipX, tipY, width * 0.5f, color) ;
			} else if ("zigzag".equals(style)) {
				double previousX = startX ;
				double previousY = startY ;
				for (int index = 1 ; index <= 5 ; index++) {
					double scale = index / 5.0 ;
					double offset = index == 5 ? 0.0 : (index % 2 == 0 ? -width * 3.0 : width * 3.0) ;
					double nextX = startX + directionX * length * scale + sideX * offset ;
					double nextY = startY + directionY * length * scale + sideY * offset ;
					drawTracerLine(g, previousX, previousY, nextX, nextY, width, color) ;
					previousX = nextX ; previousY = nextY ;
				}
			} else if ("streak".equals(style)) {
				drawTracerLine(g, startX, startY, tipX, tipY, width * 0.5f, dim) ;
				drawTracerLine(g, tipX - directionX * length * 0.35, tipY - directionY * length * 0.35,
				               tipX, tipY, width * 1.5f, color) ;
			} else if ("slug".equals(style)) {
				double slugX = tipX - directionX * length * 0.18 ;
				double slugY = tipY - directionY * length * 0.18 ;
				drawTracerLine(g, slugX, slugY, tipX, tipY, width * 3.0f, dim) ;
				drawTracerLine(g, slugX, slugY, tipX, tipY, width, color) ;
			} else if ("blade".equals(style)) {
				double radius = (5.0 + tracer.width * 3.0) * zoom ;
				for (int index = 0 ; index < 4 ; index++) {
					double bladeAngle = angle + index * Math.PI / 2.0 ;
					double outerX = tipX + Math.cos(bladeAngle) * radius ;
					double outerY = tipY + Math.sin(bladeAngle) * radius ;
					double edgeX = outerX + Math.cos(bladeAngle - 1.1) * radius * 0.65 ;
					double edgeY = outerY + Math.sin(bladeAngle - 1.1) * radius * 0.65 ;
					drawTracerLine(g, tipX, tipY, outerX, outerY, width, color) ;
					drawTracerLine(g, outerX, outerY, edgeX, edgeY, width, color) ;
				}
				drawTracerPoint(g, tipX, tipY, width, color) ;
				return ;
			} else if ("gauss".equals(style) || "charge_beam".equals(style)) {
				float glow = "charge_beam".equals(style) ? 8.0f : 6.0f ;
				drawTracerLine(g, startX, startY, tipX, tipY, width * glow, dim) ;
				drawTracerLine(g, startX, startY, tipX, tipY, width * 2.0f, color) ;
				drawTracerLine(g, startX, startY, tipX, tipY, width * 0.6f, Color.WHITE) ;
			} else if ("lightning".equals(style) || "wave".equals(style) || "heat_ray".equals(style)) {
				double previousX = startX, previousY = startY ;
				double sideScale = width * ("heat_ray".equals(style) ? 4.0 : 3.0) ;
				int phase = (int)(System.currentTimeMillis() / 90L) ;
				for (int index = 1 ; index <= 8 ; index++) {
					double scale = index / 8.0 ;
					double offset = index == 8 ? 0.0 : Math.sin((index + phase * 2) * 1.7) * sideScale ;
					if ("lightning".equals(style)) offset += ((index * 7 + phase * 3) % 5 - 2) * width ;
					double nextX = startX + directionX * length * scale + sideX * offset ;
					double nextY = startY + directionY * length * scale + sideY * offset ;
					drawTracerLine(g, previousX, previousY, nextX, nextY, width, color) ;
					previousX = nextX ; previousY = nextY ;
				}
			} else if ("spiral".equals(style)) {
				int phase = (int)(System.currentTimeMillis() / 90L) ;
				for (int strand = 0 ; strand < 2 ; strand++) {
					double previousX = startX, previousY = startY ;
					for (int index = 1 ; index <= 8 ; index++) {
						double scale = index / 8.0 ;
						double offset = Math.sin(scale * Math.PI * 4.0 + strand * Math.PI + phase * 0.8) * width * 3.0 ;
						double nextX = startX + directionX * length * scale + sideX * offset ;
						double nextY = startY + directionY * length * scale + sideY * offset ;
						drawTracerLine(g, previousX, previousY, nextX, nextY, width * 0.7f, color) ;
						previousX = nextX ; previousY = nextY ;
					}
				}
			} else if ("chain".equals(style) || "ricochet".equals(style) || "penetrator".equals(style)) {
				double previousX = startX, previousY = startY ;
				int segments = "chain".equals(style) ? 6 : 3 ;
				for (int index = 1 ; index <= segments ; index++) {
					double scale = index / (double)segments ;
					double offset = index == segments ? 0.0 : (index % 2 == 0 ? -1.0 : 1.0) * width * ("ricochet".equals(style) ? 8.0 : 2.0) ;
					double nextX = startX + directionX * length * scale + sideX * offset ;
					double nextY = startY + directionY * length * scale + sideY * offset ;
					drawTracerLine(g, previousX, previousY, nextX, nextY, width, color) ;
					if ("chain".equals(style) && index < segments) drawTracerPoint(g, nextX, nextY, width, color) ;
					previousX = nextX ; previousY = nextY ;
				}
				if ("penetrator".equals(style)) drawTracerLine(g, tipX, tipY, tipX + directionX * 12.0 * zoom, tipY + directionY * 12.0 * zoom, width * 0.7f, dim) ;
			} else if ("fading_beam".equals(style) || "expanding_beam".equals(style) || "tapered_beam".equals(style) || "gradient_beam".equals(style)) {
				int phase = (int)(System.currentTimeMillis() / 90L) % 5 ;
				for (int index = 0 ; index < 6 ; index++) {
					double from = index / 6.0, to = (index + 1) / 6.0 ;
					float segmentWidth = width ;
					Color segmentColor = color ;
					if ("fading_beam".equals(style)) segmentColor = tracerColor(tracer, tracer.alpha * (5 - phase) / 5, 1.0) ;
					else if ("expanding_beam".equals(style)) segmentWidth *= 1.0f + phase * 0.6f ;
					else if ("tapered_beam".equals(style)) segmentWidth *= 2.0f - (float)from * 1.7f ;
					else segmentColor = new Color(tracer.red + (255 - tracer.red) * index / 6,
					                                  tracer.green + (255 - tracer.green) * index / 6,
					                                  tracer.blue + (255 - tracer.blue) * index / 6, tracer.alpha) ;
					drawTracerLine(g, startX + directionX * length * from, startY + directionY * length * from,
					               startX + directionX * length * to, startY + directionY * length * to, segmentWidth, segmentColor) ;
				}
			} else if ("disruptor".equals(style) || "animated_texture".equals(style)) {
				int phase = (int)(System.currentTimeMillis() / 90L) ;
				for (int index = 0 ; index < 8 ; index++) {
					if ("disruptor".equals(style) && (index + phase) % 3 == 1) continue ;
					if ("animated_texture".equals(style) && (index + phase) % 2 == 1) continue ;
					double from = index / 8.0, to = (index + 0.7) / 8.0 ;
					drawTracerLine(g, startX + directionX * length * from, startY + directionY * length * from,
					               startX + directionX * length * to, startY + directionY * length * to, width, color) ;
				}
			} else if ("particle_trail".equals(style) || "smoke_trail".equals(style)) {
				for (int index = 0 ; index < 8 ; index++) {
					double scale = index / 7.0 ;
					float size = width * ("smoke_trail".equals(style) ? 2.0f + (float)scale * 2.0f : 1.0f) ;
					drawTracerPoint(g, tipX - directionX * length * scale, tipY - directionY * length * scale,
					                size, tracerColor(tracer, tracer.alpha * (8 - index) / 8, 1.0)) ;
				}
				return ;
			} else if ("impact_ring".equals(style) || "impact_burst".equals(style)) {
				int phase = (int)(System.currentTimeMillis() / 90L) % 5 ;
				double radius = (4.0 + phase * 2.0) * width ;
				Color impactColor = tracerColor(tracer, tracer.alpha * (5 - phase) / 5, 1.0) ;
				for (int index = 0 ; index < 8 ; index++) {
					double angle1 = index * Math.PI / 4.0, angle2 = (index + 1) * Math.PI / 4.0 ;
					if ("impact_ring".equals(style)) drawTracerLine(g, tipX + Math.cos(angle1) * radius, tipY + Math.sin(angle1) * radius,
					                                                   tipX + Math.cos(angle2) * radius, tipY + Math.sin(angle2) * radius, width, impactColor) ;
					else drawTracerLine(g, tipX + Math.cos(angle1) * radius * 0.25, tipY + Math.sin(angle1) * radius * 0.25,
					                     tipX + Math.cos(angle1) * radius, tipY + Math.sin(angle1) * radius, width, impactColor) ;
				}
				return ;
			} else if ("afterimage".equals(style)) {
				for (int index = 0 ; index < 4 ; index++) {
					double offset = (index - 1.5) * width * 2.0 ;
					drawTracerLine(g, startX + sideX * offset, startY + sideY * offset,
					               tipX + sideX * offset, tipY + sideY * offset, width,
					               tracerColor(tracer, tracer.alpha * (4 - index) / 4, 1.0)) ;
				}
			} else {
				drawTracerLine(g, startX, startY, tipX, tipY, width * 0.7f, dim) ;
				drawTracerLine(g, tipX - directionX * length * 0.75, tipY - directionY * length * 0.75,
				               tipX, tipY, width * 0.8f, color) ;
			}
			drawTracerPoint(g, tipX, tipY, Math.max(1.0f, (float)zoom * 0.5f), color) ;
		}

		Color tracerColor(TracerConfig tracer, int alpha, double brightness) {
			return new Color((int)(tracer.red * brightness), (int)(tracer.green * brightness),
			                 (int)(tracer.blue * brightness), Math.max(0, Math.min(255, alpha))) ;
		}

		void drawTracerLine(Graphics2D g, double x1, double y1, double x2, double y2, float width, Color color) {
			Stroke oldStroke = g.getStroke() ;
			g.setStroke(new BasicStroke(Math.max(1.0f, width), BasicStroke.CAP_ROUND, BasicStroke.JOIN_ROUND)) ;
			g.setColor(color) ;
			g.drawLine((int)Math.round(x1), (int)Math.round(y1), (int)Math.round(x2), (int)Math.round(y2)) ;
			g.setStroke(oldStroke) ;
		}

		void drawTracerPoint(Graphics2D g, double x, double y, float radius, Color color) {
			int size = Math.max(2, (int)Math.ceil(radius * 2.0f)) ;
			g.setColor(color) ;
			g.fillRect((int)Math.round(x - size * 0.5), (int)Math.round(y - size * 0.5), size, size) ;
		}

		boolean hasMuzzleFlash(WeaponInfo weapon) {return (weapon.id > 0 && weapon.id < 25) || weapon.id >= 28 ;}

		void drawFlash(Graphics2D g, WeaponInfo weapon, int x, int y, int zoom) {
			if (!hasMuzzleFlash(weapon)) {drawCentered(g, "none", x, y, labelColor()) ; return ;}
			BufferedImage flash = frame.model.flash(weapon.flashStyle) ;
			drawQuad(g, flash, frame.flashFrame() * 32, 0, 32, 32, x, y, 16, 16, 0, zoom, false) ;
		}

		void drawWeaponCell(Graphics2D g, BufferedImage page, int id, double x, double y,
		                    double hotX, double hotY, double angle, double zoom) {
			Rectangle cell = frame.model.cell(page, id) ;
			if (cell == null) {drawCentered(g, "missing", (int)x, (int)y, ACCENT) ; return ;}
			drawQuad(g, page, cell.x, cell.y, 32, 32, x, y, hotX, hotY, angle, zoom, false) ;
		}

		void drawQuad(Graphics2D g, BufferedImage image, int sourceX, int sourceY, int width, int height,
		              double x, double y, double hotX, double hotY, double angle, double zoom, boolean flip) {
			if (image == null) return ;
			AffineTransform old = g.getTransform() ;
			g.translate(x, y) ;
			g.rotate(angle) ;
			g.scale(flip ? -zoom : zoom, zoom) ;
			g.drawImage(image, (int)-hotX, (int)-hotY, (int)(width - hotX), (int)(height - hotY),
			            sourceX, sourceY, sourceX + width, sourceY + height, null) ;
			g.setTransform(old) ;
		}

		void drawCentered(Graphics2D g, String text, int x, int y, Color color) {
			g.setColor(color) ;
			FontMetrics metrics = g.getFontMetrics() ;
			g.drawString(text, x - metrics.stringWidth(text) / 2, y + metrics.getAscent() / 2) ;
		}
	}
}
