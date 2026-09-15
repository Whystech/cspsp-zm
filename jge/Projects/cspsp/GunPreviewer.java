import java.awt.* ;
import java.awt.event.* ;
import java.awt.geom.AffineTransform ;
import java.awt.image.BufferedImage ;
import java.io.* ;
import java.net.URISyntaxException ;
import java.util.ArrayList ;
import java.util.HashMap ;
import java.util.List ;
import java.util.Map ;
import java.util.prefs.Preferences ;
import javax.imageio.ImageIO ;
import javax.swing.* ;
import javax.swing.border.EmptyBorder ;
import javax.swing.filechooser.FileNameExtensionFilter ;

public class GunPreviewer {
	static final int MAX_MUZZLE_FLASH_TYPES = 128 ;
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
		int damage, delay, clip, numClips, reloadDelay, cost, type, fireMode, pellets, scope, category, teams, flashStyle ;
		float spread, walkingSpeed, bulletSpeed, viewAngle ;
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
			name = fields[fields.length - 1] ;
		}

		String record() {
			return id + " " + damage + " " + delay + " " + decimal(spread) + " " + clip + " " + numClips
			       + " " + reloadDelay + " " + decimal(walkingSpeed) + " " + decimal(bulletSpeed) + " "
			       + decimal(viewAngle) + " " + cost + " " + type + " " + fireMode + " " + pellets + " "
			       + scope + " " + category + " " + teams + " " + flashStyle + " " + name ;
		}

		static String decimal(float value) {return Float.toString(value) ;}

		static WeaponInfo createDefault(int id) {
			return new WeaponInfo((id + " 20 100 0.3 30 5 2000 1.0 1.0 0.0 1000 0 1 1 0 4 3 0 NEW-GUN").split(" ")) ;
		}

		String weaponType() {
			String[] names = {"PRIMARY", "SECONDARY", "KNIFE", "GRENADE"} ;
			return type >= 0 && type < names.length ? names[type] : "TYPE " + type ;
		}

		String bulletType() {
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

	private static class TracerShot {
		final int weaponId ;
		final long startedAt ;
		final double angle ;

		TracerShot(int weaponId, long startedAt, double angle) {
			this.weaponId = weaponId ; this.startedAt = startedAt ; this.angle = angle ;
		}
	}

	static class ResourceModel {
		static final Preferences PREFERENCES = Preferences.userNodeForPackage(GunPreviewer.class) ;
		final List<WeaponInfo> weapons = new ArrayList<WeaponInfo>() ;
		final BufferedImage[] handPages = new BufferedImage[2] ;
		final BufferedImage[] groundPages = new BufferedImage[2] ;
		final Map<Integer, BufferedImage> flashes = new HashMap<Integer, BufferedImage>() ;
		final Map<Integer, TracerConfig> tracers = new HashMap<Integer, TracerConfig>() ;
		final Map<Integer, LaserConfig> lasers = new HashMap<Integer, LaserConfig>() ;
		BufferedImage players ;
		File directory ;
		File graphicsDirectory ;
		File dataDirectory ;
		File requestedRoot ;
		final List<String> trailingLines = new ArrayList<String>() ;
		final List<String> tracerLines = new ArrayList<String>() ;
		final List<String> laserLines = new ArrayList<String>() ;

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
			trailingLines.clear() ;
			tracerLines.clear() ;
			laserLines.clear() ;
			handPages[0] = readRequired("guns.png") ;
			handPages[1] = readOptional("guns2.png") ;
			groundPages[0] = readRequired("gunsground.png") ;
			groundPages[1] = readOptional("gunsground2.png") ;
			players = readRequired("players.png") ;
			loadMuzzleFlashes() ;
			validateAtlas(handPages[0], "guns.png") ;
			validateAtlas(groundPages[0], "gunsground.png") ;
			if (handPages[1] != null) validateAtlas(handPages[1], "guns2.png") ;
			if (groundPages[1] != null) validateAtlas(groundPages[1], "gunsground2.png") ;
			loadWeapons() ;
			loadTracers() ;
			loadLasers() ;
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

		void saveWeapons() throws IOException {
			File target = new File(dataDirectory, "guns.txt") ;
			File temporary = new File(dataDirectory, "guns.txt.tmp") ;
			PrintWriter writer = new PrintWriter(new OutputStreamWriter(new FileOutputStream(temporary), "UTF-8")) ;
			try {
				writer.println(weapons.size()) ;
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
		                                                "flare", "streak", "slug", "blade"} ;
		final PreviewFrame frame ;
		final WeaponInfo weapon ;
		final boolean newWeapon ;
		final TracerConfig tracer ;
		final LaserConfig laser ;
		File pendingHandSprite, pendingGroundSprite ;
		final JTextField name = new JTextField() ;
		final JSpinner damage = integer(0, 10000), delay = integer(0, 60000), clip = integer(0, 10000) ;
		final JSpinner numClips = integer(0, 1000), reloadDelay = integer(0, 60000), cost = integer(0, 100000) ;
		final JSpinner pellets = integer(1, 1000), flashStyle = integer(0, MAX_MUZZLE_FLASH_TYPES - 1) ;
		final JSpinner tracerRed = integer(0, 255), tracerGreen = integer(0, 255), tracerBlue = integer(0, 255), tracerAlpha = integer(0, 255) ;
		final JSpinner spread = decimal(0.0, 100.0, 0.01), walkingSpeed = decimal(0.0, 100.0, 0.05) ;
		final JSpinner bulletSpeed = decimal(0.0, 100.0, 0.05), viewAngle = decimal(0.0, 10.0, 0.01) ;
		final JSpinner tracerLength = decimal(0.0, 10000.0, 1.0), tracerWidth = decimal(0.1, 100.0, 0.1) ;
		final JSpinner laserRed = integer(0, 255), laserGreen = integer(0, 255), laserBlue = integer(0, 255), laserAlpha = integer(0, 255) ;
		final JSpinner laserWidth = decimal(0.1, 20.0, 0.1), laserRange = decimal(1.0, 5000.0, 10.0) ;
		final JSpinner laserFalloff = decimal(0.0, 10.0, 0.1), laserEndDotScale = decimal(0.1, 20.0, 0.1) ;
		final JSpinner laserOffsetX = decimal(-100.0, 100.0, 0.5), laserOffsetY = decimal(-100.0, 100.0, 0.5) ;
		final JCheckBox laserEnabled = new JCheckBox("Enabled"), laserFalloffEnabled = new JCheckBox("Use intensity falloff") ;
		final JCheckBox laserEndDot = new JCheckBox("Display endpoint dot") ;
		final JComboBox<String> type = new JComboBox<String>(new String[] {"Primary", "Secondary", "Knife", "Grenade"}) ;
		final JComboBox<String> fireMode = new JComboBox<String>(new String[] {"Semi-auto", "Automatic"}) ;
		final JComboBox<String> scope = new JComboBox<String>(new String[] {"None", "Low", "Medium", "High"}) ;
		final JComboBox<String> category = new JComboBox<String>(new String[] {"Hidden", "Pistols", "Shotguns", "SMG", "Rifles", "Machine guns", "Equipment"}) ;
		final JComboBox<String> teams = new JComboBox<String>(new String[] {"None", "Terrorist", "Counter-Terrorist", "Both"}) ;
		final JComboBox<String> tracerStyle = new JComboBox<String>(TRACER_STYLES) ;
		final TracerPreviewPanel tracerPreview = new TracerPreviewPanel() ;
		final ColorSwatch tracerColorSwatch = new ColorSwatch() ;
		final LaserPreviewPanel laserPreview = new LaserPreviewPanel() ;
		final LaserColorSwatch laserColorSwatch = new LaserColorSwatch() ;

		WeaponEditorDialog(PreviewFrame frame, WeaponInfo weapon) {this(frame, weapon, false) ;}

		WeaponEditorDialog(PreviewFrame frame, WeaponInfo weapon, boolean newWeapon) {
			super(frame, (newWeapon ? "Add weapon " : "Configure ") + weapon.name, true) ;
			this.frame = frame ;
			this.weapon = weapon ;
			this.newWeapon = newWeapon ;
			this.tracer = frame.model.tracer(weapon.id) ;
			this.laser = frame.model.laser(weapon.id) ;
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
			JPanel tracerPreviewArea = new JPanel(new BorderLayout(12, 0)) ;
			tracerPreviewArea.setBorder(BorderFactory.createTitledBorder("Live tracer preview")) ;
			tracerPreviewArea.add(tracerPreview, BorderLayout.CENTER) ;
			tracerPreviewArea.add(colorPicker, BorderLayout.EAST) ;
			JPanel tracerPage = new JPanel(new BorderLayout(0, 10)) ;
			tracerPage.setBorder(new EmptyBorder(10, 10, 10, 10)) ;
			tracerPage.add(tracerPreviewArea, BorderLayout.NORTH) ;
			tracerPage.add(scrollable(tracerFields), BorderLayout.CENTER) ;

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
			JPanel laserPage = new JPanel(new BorderLayout(0, 10)) ;
			laserPage.setBorder(new EmptyBorder(10, 10, 10, 10)) ;
			laserPage.add(laserPreviewArea, BorderLayout.NORTH) ;
			laserPage.add(scrollable(laserFields), BorderLayout.CENTER) ;

			JTabbedPane pages = new JTabbedPane() ;
			pages.addTab("1. Weapon", weaponPage) ;
			pages.addTab("2. Tracer", tracerPage) ;
			pages.addTab("3. Laser", laserPage) ;
			content.add(pages, BorderLayout.CENTER) ;
			JPanel actions = new JPanel(new FlowLayout(FlowLayout.RIGHT)) ;
			JButton cancel = new JButton("Cancel") ;
			cancel.addActionListener(event -> dispose()) ;
			JButton save = new JButton(newWeapon ? "Add gun" : "Save guns.txt") ;
			save.addActionListener(event -> save()) ;
			actions.add(cancel) ;
			actions.add(save) ;
			content.add(actions, BorderLayout.SOUTH) ;
			setContentPane(content) ;
			loadValues() ;
			installTracerPreviewListeners() ;
			installLaserPreviewListeners() ;
			setSize(720, 620) ;
			setMinimumSize(new Dimension(640, 500)) ;
			setLocationRelativeTo(frame) ;
		}

		static JSpinner integer(int minimum, int maximum) {return new JSpinner(new SpinnerNumberModel(minimum, minimum, maximum, 1)) ;}
		static JSpinner decimal(double minimum, double maximum, double step) {return new JSpinner(new SpinnerNumberModel(minimum, minimum, maximum, step)) ;}

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
			return label ;
		}

		void loadValues() {
			name.setText(weapon.name) ; damage.setValue(weapon.damage) ; delay.setValue(weapon.delay) ;
			spread.setValue((double)weapon.spread) ; clip.setValue(weapon.clip) ; numClips.setValue(weapon.numClips) ;
			reloadDelay.setValue(weapon.reloadDelay) ; walkingSpeed.setValue((double)weapon.walkingSpeed) ;
			bulletSpeed.setValue((double)weapon.bulletSpeed) ; viewAngle.setValue((double)weapon.viewAngle) ; cost.setValue(weapon.cost) ;
			type.setSelectedIndex(weapon.type) ; fireMode.setSelectedIndex(weapon.fireMode) ; pellets.setValue(weapon.pellets) ;
			scope.setSelectedIndex(weapon.scope) ; category.setSelectedIndex(weapon.category) ; teams.setSelectedIndex(weapon.teams) ;
			flashStyle.setValue(weapon.flashStyle) ;
			tracerStyle.setSelectedItem(tracer.style) ;
			tracerRed.setValue(tracer.red) ; tracerGreen.setValue(tracer.green) ; tracerBlue.setValue(tracer.blue) ;
			tracerAlpha.setValue(tracer.alpha) ; tracerLength.setValue((double)tracer.length) ; tracerWidth.setValue((double)tracer.width) ;
			laserEnabled.setSelected(laser.enabled) ; laserFalloffEnabled.setSelected(laser.falloffEnabled) ;
			laserRed.setValue(laser.red) ; laserGreen.setValue(laser.green) ; laserBlue.setValue(laser.blue) ; laserAlpha.setValue(laser.alpha) ;
			laserWidth.setValue((double)laser.width) ; laserRange.setValue((double)laser.range) ; laserFalloff.setValue((double)laser.falloff) ;
			laserEndDot.setSelected(laser.endDot) ; laserEndDotScale.setValue((double)laser.endDotScale) ;
			laserOffsetX.setValue((double)laser.offsetX) ; laserOffsetY.setValue((double)laser.offsetY) ;
			refreshTracerPreview() ;
			refreshLaserPreview() ;
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
			LaserPreviewPanel() {setPreferredSize(new Dimension(430, 110)) ; setBackground(BACKGROUND) ;}
			protected void paintComponent(Graphics graphics) {
				super.paintComponent(graphics) ;
				Graphics2D g = (Graphics2D)graphics.create() ;
				try {
					g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON) ;
					LaserConfig preview = previewLaser() ;
					if (!preview.enabled) {frame.canvas.drawCentered(g, "disabled", getWidth()/2, getHeight()/2, MUTED) ; return ;}
					double originX = 42.0, originY = getHeight()/2.0 ;
					g.setColor(new Color(110, 116, 124)) ;
					g.drawLine((int)originX-5, (int)originY, (int)originX+5, (int)originY) ;
					g.drawLine((int)originX, (int)originY-5, (int)originX, (int)originY+5) ;
					double startX = originX+preview.offsetX*2.0, y = originY+preview.offsetY*2.0 ;
					double endX = Math.min(getWidth()-18.0, startX+Math.max(20.0, preview.range*0.45)) ;
					int segments = preview.falloffEnabled && preview.falloff > 0.0f ? 12 : 1 ;
					for (int index=0 ; index<segments ; index++) {
						double from = index/(double)segments, to = (index+1)/(double)segments ;
						double intensity = segments == 1 ? 1.0 : Math.pow(Math.max(0.0, 1.0-(from+to)*0.5), preview.falloff) ;
						Color color = new Color(preview.red, preview.green, preview.blue, (int)(preview.alpha*intensity)) ;
						frame.canvas.drawTracerLine(g,startX+(endX-startX)*from,y,startX+(endX-startX)*to,y,preview.width,color) ;
					}
					if (preview.endDot) {
						double intensity = preview.falloffEnabled && preview.falloff > 0.0f ? 0.15 : 1.0 ;
						frame.canvas.drawTracerPoint(g,endX,y,preview.endDotScale,new Color(preview.red,preview.green,preview.blue,(int)(preview.alpha*intensity))) ;
					}
					double[] gunAnchor = frame.canvas.idleGunAnchor(weapon) ;
					frame.canvas.drawWeaponCell(g,frame.model.handPage(weapon.id),weapon.id,
					                            originX+gunAnchor[0]*2.0,originY+gunAnchor[1]*2.0,
					                            16,8,-Math.PI/2.0,2.0) ;
				} finally {g.dispose() ;}
			}
		}

		void installTracerPreviewListeners() {
			tracerStyle.addActionListener(event -> refreshTracerPreview()) ;
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
					g.setColor(new Color(25, 28, 32)) ;
					for (int x = 0 ; x < getWidth() ; x += 20) g.drawLine(x, 0, x, getHeight()) ;
					for (int y = 0 ; y < getHeight() ; y += 20) g.drawLine(0, y, getWidth(), y) ;
					Rectangle bounds = new Rectangle(8, 8, Math.max(1, getWidth()-16), Math.max(1, getHeight()-16)) ;
					frame.canvas.drawTracerCentered(g, previewTracer(), weapon, bounds, 0.0, 2) ;
				} finally {g.dispose() ;}
			}
		}

		int integer(JSpinner spinner) {return ((Number)spinner.getValue()).intValue() ;}
		float decimal(JSpinner spinner) {return ((Number)spinner.getValue()).floatValue() ;}

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
			tracer.style = (String)tracerStyle.getSelectedItem() ;
			tracer.red = integer(tracerRed) ; tracer.green = integer(tracerGreen) ; tracer.blue = integer(tracerBlue) ;
			tracer.alpha = integer(tracerAlpha) ; tracer.length = decimal(tracerLength) ; tracer.width = decimal(tracerWidth) ;
			laser.enabled = laserEnabled.isSelected() ; laser.falloffEnabled = laserFalloffEnabled.isSelected() ;
			laser.red = integer(laserRed) ; laser.green = integer(laserGreen) ; laser.blue = integer(laserBlue) ; laser.alpha = integer(laserAlpha) ;
			laser.width = decimal(laserWidth) ; laser.range = decimal(laserRange) ; laser.falloff = decimal(laserFalloff) ;
			laser.endDot = laserEndDot.isSelected() ; laser.endDotScale = decimal(laserEndDotScale) ;
			laser.offsetX = decimal(laserOffsetX) ; laser.offsetY = decimal(laserOffsetY) ;
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
		final JComboBox<WeaponInfo> weaponBox = new JComboBox<WeaponInfo>() ;
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
		long nextShotAt, reloadStartedAt, reloadUntil ;
		boolean triggerHeld, reloading ;
		final List<TracerShot> tracerShots = new ArrayList<TracerShot>() ;

		PreviewFrame() {
			super("CSPSP Weapon Configurator & Previewer") ;
			setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE) ;
			setMinimumSize(new Dimension(960, 640)) ;
			setSize(1120, 720) ;
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
			sidebar.setBorder(new EmptyBorder(20, 18, 20, 18)) ;
			sidebar.setPreferredSize(new Dimension(270, 950)) ;
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
			addControl(sidebar, "WEAPON", weaponBox) ;
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
					WeaponInfo weapon = selectedWeapon() ;
					if (weapon != null) new WeaponEditorDialog(PreviewFrame.this, weapon).setVisible(true) ;
				}
			}) ;
			sidebar.add(configure) ;
			sidebar.add(Box.createVerticalStrut(8)) ;
			JButton addGun = button("ADD GUN", ACCENT) ;
			addGun.addActionListener(event -> addGun()) ;
			sidebar.add(addGun) ;
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
			scroll.setPreferredSize(new Dimension(290, 620)) ;
			scroll.getViewport().setBackground(PANEL) ;
			return scroll ;
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
			control.setMinimumSize(new Dimension(180, 28)) ;
			control.setPreferredSize(new Dimension(214, 28)) ;
			control.setMaximumSize(new Dimension(Integer.MAX_VALUE, 28)) ;
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
			weaponBox.addActionListener(action) ;
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
				weaponBox.removeAllItems() ;
				int selection = 0 ;
				for (int index = 0 ; index < model.weapons.size() ; index++) {
					WeaponInfo weapon = model.weapons.get(index) ;
					weaponBox.addItem(weapon) ;
					if (weapon.id == selectedId) selection = index ;
				}
				if (weaponBox.getItemCount() > 0) weaponBox.setSelectedIndex(selection) ;
				resetSimulation() ;
				selectionChanged() ;
			} catch (Exception exception) {
				status.setText("File error: " + exception.getMessage()) ;
				JOptionPane.showMessageDialog(this, exception.getMessage(), "File error", JOptionPane.ERROR_MESSAGE) ;
			}
		}

		void addGun() {
			int id = model.weapons.size() ;
			if (id >= 128) {
				JOptionPane.showMessageDialog(this, "The maximum of 128 weapons has been reached.", "Cannot add gun", JOptionPane.ERROR_MESSAGE) ;
				return ;
			}
			new WeaponEditorDialog(this, WeaponInfo.createDefault(id), true).setVisible(true) ;
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
			if (weapon != null && weapon.id != simulatedWeaponId) resetSimulation() ;
			boolean guns = previewBox.getSelectedIndex() == 0 ;
			weaponBox.setEnabled(guns) ;
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
			if (weapon.type == 2 || weapon.type == 3 || "none".equals(model.tracer(weapon.id).style)) return ;
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

		WeaponInfo selectedWeapon() {return (WeaponInfo)weaponBox.getSelectedItem() ;}
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
				drawQuad(g, flash, frame.flashFrame() * 32, 0, 32, 32, gunX, gunY, 16, -16, spriteAngle, gunZoom[0], false) ;
			}
			drawLiveTracers(g, weapon, centerX, centerY) ;
			String metadata = weapon.weaponType() + " / " + weapon.bulletType() + " / SPEED " + weapon.bulletSpeed ;
			drawCentered(g, metadata, (int)centerX, gunPanels[0].y + 48, labelColor()) ;
			drawWeaponCell(g, frame.model.groundPage(weapon.id), weapon.id, gunPanels[1].getCenterX(), gunPanels[1].getCenterY() + 8, 16, 16, 0, gunZoom[1]) ;
			drawWeaponCell(g, frame.model.handPage(weapon.id), weapon.id, gunPanels[2].getCenterX(), gunPanels[2].getCenterY() + 8, 16, 16, 0, gunZoom[2]) ;
			drawFlash(g, weapon, (int)gunPanels[3].getCenterX(), (int)gunPanels[3].getCenterY() + 8, gunZoom[3]) ;
		}

		double[] idleGunAnchor(WeaponInfo weapon) {
			double body, rightArm, rightHand ;
			if (weapon.type == 0) {body=30.0 ; rightArm=15.0 ; rightHand=-60.0 ;}
			else if (weapon.type == 1) {body=5.0 ; rightArm=-10.0 ; rightHand=-40.0 ;}
			else if (weapon.type == 2) {body=0.0 ; rightArm=20.0 ; rightHand=-40.0 ;}
			else {body=0.0 ; rightArm=20.0 ; rightHand=-30.0 ;}
			body = Math.toRadians(body) ;
			rightArm = Math.toRadians(rightArm) ;
			rightHand = Math.toRadians(rightHand) ;
			return new double[] {
				-5.0-10.0*Math.sin(body)+8.0*Math.cos(rightArm)+10.0*Math.cos(rightHand),
				10.0*Math.cos(body)+8.0*Math.sin(rightArm)+10.0*Math.sin(rightHand)
			} ;
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
			if ("none".equals(tracer.style)) return ;
			long now = System.currentTimeMillis() ;
			Shape oldClip = g.getClip() ;
			g.clip(gunPanels[0]) ;
			for (TracerShot shot : frame.tracerShots) {
				if (shot.weaponId != weapon.id) continue ;
				double elapsed = now - shot.startedAt ;
				double distance = (24.0 + 0.3 * weapon.bulletSpeed * elapsed) * gunZoom[0] ;
				double tipX = centerX + Math.cos(shot.angle) * distance ;
				double tipY = centerY + Math.sin(shot.angle) * distance ;
				double distanceFromMuzzle = Math.max(0.0, distance - 24.0 * gunZoom[0]) ;
				drawTracer(g, tracer, tipX, tipY, shot.angle, gunZoom[0], weapon, distanceFromMuzzle) ;
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
