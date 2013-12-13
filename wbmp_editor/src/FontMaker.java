import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.GraphicsEnvironment;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;

import javax.imageio.ImageIO;


public class FontMaker {
	
	private static final int HEIGHT = 16;
	private static final int WIDTH = 10;
	private static final int FONT_SIZE = 16;
	private static final int FONT_YOFFSET = 15;
	private static final String FONT_NAME = "Minecraftia";
	
	private static final char CHAR_START = 0x20;
	private static final char CHAR_LAST = 0x60;
	
	public static void main(String[] args) throws Exception {
		//getAllFonts();
		//testFont((char) 0x60);
		produceFile("font_"+FONT_NAME+HEIGHT+".h");
	}
	
	public static void produceFile(final String filename) throws Exception {
		final int chars = CHAR_LAST - CHAR_START + 1;
		final StringBuilder b = new StringBuilder("#ifndef __FONT_"+FONT_NAME+HEIGHT+"\n#define __FONT_"+FONT_NAME+HEIGHT+"\n#include \"lcd.h\"\n\n");
		b.append("static const bitmap_t font_"+FONT_NAME+HEIGHT+" = {"+WIDTH+","+(HEIGHT/8)+","+chars+","+(int) CHAR_START+",{");
		for (char c = CHAR_START; c >= CHAR_START; c++) {
			b.append(genDrawerFor(c).getRawCCode());
			b.append(",\n");
		}
		b.append("}};\n\n#endif\n");
		
		PrintWriter out = new PrintWriter(filename);
		out.print(b.toString());
		out.close();
	}
	
	public static Drawer genDrawerFor(final char c) throws Exception {
		final String text = String.valueOf(c);
		
		final BufferedImage bi = new BufferedImage(WIDTH, HEIGHT, BufferedImage.TYPE_3BYTE_BGR);
		final Graphics2D g = bi.createGraphics();
		g.setRenderingHint(
		        RenderingHints.KEY_TEXT_ANTIALIASING,
		        RenderingHints.VALUE_TEXT_ANTIALIAS_LCD_HRGB);
		
		
		g.setColor(Color.white);
		g.fillRect(0, 0, WIDTH, HEIGHT);
		g.setColor(Color.black);

		final Font font = new Font(FONT_NAME, Font.PLAIN, FONT_SIZE);
		g.setFont(font);
		g.drawString(text, 0, FONT_YOFFSET);
		
		g.dispose();

		final Drawer d = new Drawer();
		d.fromWhitePicture(bi);
		return d;
	}
	
	public static void testFont(final char c) throws Exception {
		genDrawerFor(c).show();
	}
	
	public static void getAllFonts() throws IOException {
		final String fontNames[] = GraphicsEnvironment.getLocalGraphicsEnvironment().getAvailableFontFamilyNames();
		
		final BufferedImage bi = new BufferedImage(300, HEIGHT * fontNames.length, BufferedImage.TYPE_3BYTE_BGR);
		final Graphics2D g = bi.createGraphics();
		
		g.setColor(Color.white);
		g.fillRect(0, 0, 300, HEIGHT * fontNames.length);
		g.setColor(Color.black);
		
		for (int i = 0; i < fontNames.length; i++) {
			final String text = fontNames[i];
			g.setFont(new Font(text, Font.PLAIN, FONT_SIZE));
			g.drawString(text, 0, i*HEIGHT+FONT_YOFFSET);
		}
		
		g.dispose();
		
		ImageIO.write(bi, "png", new File("fonts.png"));
	}

}
