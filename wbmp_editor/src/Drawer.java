import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.image.BufferedImage;
import java.util.ArrayList;

import javax.swing.JFrame;
import javax.swing.JPanel;


public class Drawer extends JPanel {
	
	private static final long serialVersionUID = -504226944187434101L;
	private int imwidth, imheight, screenwidth, screenheight, pixelsizeint;
	private double pixelsize;
	boolean[][] buffer = null;
	private final ArrayList<OnChangedCallback> callbacks = new ArrayList<Drawer.OnChangedCallback>();
	
	private final static String xseparator = ",";

	public Drawer() {
		addMouseListener(new MouseListener() {
			public void mouseReleased(MouseEvent arg0) {}
			public void mousePressed(MouseEvent arg0) {}
			public void mouseExited(MouseEvent arg0) {}
			public void mouseEntered(MouseEvent arg0) {}
			
			@Override
			public void mouseClicked(MouseEvent e) {
				
				if (buffer == null || imwidth < 0 || imheight < 0) return;
				
				final int x = (int) (e.getX() / pixelsize);
				final int y = (int) (e.getY() / pixelsize);
				
				if (x < 0 || x >= imwidth || y < 0 || y >= imheight) return;
				
				final int b1 = MouseEvent.BUTTON1_DOWN_MASK;
				final int b2 = MouseEvent.BUTTON2_DOWN_MASK;
				final boolean oldval = buffer[y][x];
				buffer[y][x] = (e.getModifiersEx() & (b1 | b2)) == b1;
				if (oldval != buffer[y][x]) announceChanged();
			}
		});
		
		addMouseMotionListener(new MouseMotionListener() {
			public void mouseMoved(MouseEvent arg0) {}
			
			@Override
			public void mouseDragged(MouseEvent e) {
				if (buffer == null || imwidth < 0 || imheight < 0) return;
				
				final int x = (int) (e.getX() / pixelsize);
				final int y = (int) (e.getY() / pixelsize);
				
				if (x < 0 || x >= imwidth || y < 0 || y >= imheight) return;
				
				final int b1 = MouseEvent.BUTTON1_DOWN_MASK;
				final int b2 = MouseEvent.BUTTON2_DOWN_MASK;
				final boolean oldval = buffer[y][x];
				buffer[y][x] = (e.getModifiersEx() & (b1 | b2)) == b1;
				if (oldval != buffer[y][x]) announceChanged();
			}
		});
	}
	
	public void registerCallback(OnChangedCallback callback) {
		callbacks.add(callback);
	}
	
	public void unregisterCallback(OnChangedCallback callback) {
		callbacks.remove(callback);
	}
	
	private void announceChanged() {
		for (final OnChangedCallback callback : callbacks)
			callback.onChanged(buffer, imwidth, imheight);
		repaint();
	}
	
	public void setSize(int width, int height) {
		
		this.imwidth = width;
		this.imheight = height;
		
		if (width <= 0 || height <= 0) {
			buffer = null;
			return;
		}
		
		buffer = new boolean[height][];
		for (int i = 0; i < height; i++)
			buffer[i] = new boolean[width];
		
		pixelsize = Math.min(screenwidth / (double) imwidth, screenheight / (double) imheight);
		pixelsizeint = (int) (1+pixelsize);
		
		announceChanged();
	}
	
	@Override
    public void setBounds(int x, int y, int width, int height) {
		this.screenwidth = width;
		this.screenheight = height;
		
		pixelsize = Math.min(screenwidth / (double) imwidth, screenheight / (double) imheight);
		pixelsizeint = (int) (1+pixelsize);
		
		super.setBounds(x, y, width, height);
	}
	
	@Override
	public void paint(Graphics g) {
		super.paint(g);
		
		g.clearRect(0, 0, screenwidth, screenheight);
		if (buffer == null || imwidth < 0 || imheight < 0) return;
		
		final int realwidth = (int) (imwidth*pixelsize);
		final int realheight = (int) (imheight*pixelsize);
		
		g.setColor(Color.white);
		g.fillRect(0, 0, realwidth, realheight);
		g.setColor(Color.black);
		
		
		for (int y = 0; y < imheight; y++)
			for (int x = 0; x < imwidth; x++)
				if (buffer[y][x])
					g.fillRect((int) (x*pixelsize), (int) (y*pixelsize), pixelsizeint, pixelsizeint);
				
	}
	
	public String getRawCCode() {
		final int rows = imheight / 8;
		final StringBuilder result = new StringBuilder();
		
		final int lastrow = rows-1;
		final int lastcol = imwidth-1;
		
		for (int row = 0; row < rows; row++) {
			
			final int rowpx = row << 3;
			final boolean[] r0 = buffer[rowpx];
			final boolean[] r1 = buffer[rowpx+1];
			final boolean[] r2 = buffer[rowpx+2];
			final boolean[] r3 = buffer[rowpx+3];
			final boolean[] r4 = buffer[rowpx+4];
			final boolean[] r5 = buffer[rowpx+5];
			final boolean[] r6 = buffer[rowpx+6];
			final boolean[] r7 = buffer[rowpx+7];
			
			for (int x = 0; x < imwidth; x++) {
				int val = 0;
				
				if (r0[x]) val = 1;
				if (r1[x]) val |= 1 << 1;
				if (r2[x]) val |= 1 << 2;
				if (r3[x]) val |= 1 << 3;
				if (r4[x]) val |= 1 << 4;
				if (r5[x]) val |= 1 << 5;
				if (r6[x]) val |= 1 << 6;
				if (r7[x]) val |= 1 << 7;
				
				result.append(val);
				if (row != lastrow || x != lastcol) result.append(xseparator);
			}
		}
		
		return result.toString();
	}

	public String getCCode() {
		if (buffer == null || imheight <= 0 || imwidth <= 0) return "Image not initialized!";
		if (imheight % 8 != 0) return "Height must be multiple of 8!";
		
		final StringBuilder result = new StringBuilder("static const bitmap_t untitled = {");
		final int rows = imheight / 8;
		result.append(imwidth);
		result.append(xseparator);
		result.append(rows);
		result.append(xseparator);
		result.append("{");
		
		result.append(getRawCCode());
		
		result.append("}};\n");
		return result.toString();
	}
	
	public String[] extractBracketedElements(final String input) throws Exception {
		final char[] istring = input.toCharArray();
		StringBuilder buf = new StringBuilder();
		final ArrayList<String> items = new ArrayList<String>();
		
		int depth = 0;
		for (int i = 0; i < istring.length; i++) {
			final char c = istring[i];
			
			if (c == '{') {
				if (depth > 0) buf.append(c);
				depth++;
			} else if (c == '}') {
				depth--;
				if (depth > 0) buf.append(c);
			} else if (c == ',' && depth == 1) {
				items.add(buf.toString().trim());
				buf = new StringBuilder();
			} else if (depth > 0)
				buf.append(c);
			
			
			
			if (depth < 0)
				throw new Exception("More closing brackets than opening ones!");
		}
		
		if (depth != 0)
			throw new Exception("More opening brackets than closing ones!");
		
		items.add(buf.toString().trim());
		
		return items.toArray(new String[items.size()]);
		
	}
	
	private int parseInt(final String text) {
		if (text.startsWith("0x"))
			return Integer.valueOf(text.substring(2), 16);
		else
			return Integer.valueOf(text);
	}
	
	public void fromCCode(final String source) throws Exception {
		final String[] meta = extractBracketedElements(source);
		final int imwidth = Integer.valueOf(meta[0]);
		final int imlines = Integer.valueOf(meta[1]);
		final String[] data = extractBracketedElements(meta[2]);
		
		final int[][] numbers = new int[imlines][];
		
		int id = 0;
		for (int row = 0; row < imlines; row++) {

			numbers[row] = new int[imwidth];
			final int[] rowdata = numbers[row];
			
			for (int x = 0; x < imwidth; x++)
				rowdata[x] = parseInt(data[id++]);
		}
		
		setSize(numbers[0].length, imlines*8);
		
		for (int row = 0; row < imlines; row++) {
			final int rowpx = row << 3;
			final boolean[] r0 = buffer[rowpx];
			final boolean[] r1 = buffer[rowpx+1];
			final boolean[] r2 = buffer[rowpx+2];
			final boolean[] r3 = buffer[rowpx+3];
			final boolean[] r4 = buffer[rowpx+4];
			final boolean[] r5 = buffer[rowpx+5];
			final boolean[] r6 = buffer[rowpx+6];
			final boolean[] r7 = buffer[rowpx+7];
			
			for (int x = 0; x < imwidth; x++) {
				int val = numbers[row][x];
				
				r0[x] = ( val & 1) != 0;
				r1[x] = ( val & (1<<1)) != 0;
				r2[x] = ( val & (1<<2)) != 0;
				r3[x] = ( val & (1<<3)) != 0;
				r4[x] = ( val & (1<<4)) != 0;
				r5[x] = ( val & (1<<5)) != 0;
				r6[x] = ( val & (1<<6)) != 0;
				r7[x] = ( val & (1<<7)) != 0;

			}
		}
		
		announceChanged();
	}
	
	public void fromPicture(final BufferedImage im, final int numbe_halftones) throws Exception {
		if (im == null) throw new Exception("No image supplied");
		final int height = im.getHeight();
		
		setSize(im.getWidth(), (int) Math.ceil(height/8)*8);
		
	    final int c = (int)(0.001 + Math.sqrt(numbe_halftones));
	    final int cn = 256/numbe_halftones;
	    
	    for (int y=0; y<imheight-c+1; y+=c) {
	    	for (int x=0; x<imwidth-c+1; x+=c) {
	    		final Color col = new Color(im.getRGB(x, y));
	    		final int val = (col.getRed() + col.getGreen() + col.getBlue() ) / 3;
	    		int count = val/cn;
	    		for (int m=y; m<y+c && count!=0; m++) {
	    			for (int n=x; n<x+c && count!=0; n++) {
	    				buffer[m][n] =true;
	    				count--;
	    			}
	    		}
	    	}
	    }
		
		announceChanged();
	}
	
	public void fromWhitePicture(final BufferedImage im) throws Exception {
		if (im == null) throw new Exception("No image supplied");
		final int height = im.getHeight();
		
		setSize(im.getWidth(), (int) Math.ceil(height/8)*8);
	    
	    for (int y=0; y<imheight; y++) {
	    	for (int x=0; x<imwidth; x++) {
	    		final Color col = new Color(im.getRGB(x, y));
	    		final int val = (col.getRed() + col.getGreen() + col.getBlue() ) / 3;
	    		buffer[y][x] = val < 128;
	    	}
	    }
		
		announceChanged();
	}
	
	public static interface OnChangedCallback {
		void onChanged(final boolean[][] buffer, final int imwidth, final int imheight);
	}
	
	public void show() {
		setPreferredSize (new Dimension (911, 686));
        setLayout (null);
        
		final JFrame frame = new JFrame ("Display of Drawer");
        frame.setDefaultCloseOperation (JFrame.EXIT_ON_CLOSE);
        frame.getContentPane().add (this);
        frame.pack();
        frame.setVisible (true);
	}
}
