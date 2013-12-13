import java.awt.Color;
import java.awt.Graphics;

import javax.swing.JPanel;


public class Preview extends JPanel implements Drawer.OnChangedCallback {

	private static final long serialVersionUID = -8907138114391916581L;
	private int imwidth, imheight, screenwidth, screenheight;
	boolean[][] buffer = null;
	
	@Override
	public void paint(Graphics g) {
		super.paint(g);
		
		g.setColor(Color.white);
		g.clearRect(0, 0, screenwidth, screenheight);
		g.fillRect(0, 0, imwidth, imheight);
		g.setColor(Color.black);
		
		if (buffer == null || imwidth < 0 || imheight < 0) return;
		
		for (int y = 0; y < imheight; y++)
			for (int x = 0; x < imwidth; x++)
				if (buffer[y][x])
					g.fillRect(x, y, 1, 1);
				
	}
	
	@Override
    public void setBounds(int x, int y, int width, int height) {
		this.screenwidth = width;
		this.screenheight = height;
		
		super.setBounds(x, y, width, height);
	}

	@Override
	public void onChanged(boolean[][] buffer, int imwidth, int imheight) {
		if (buffer == null || imwidth <= 0 || imheight <= 0) return;
		
		if (this.buffer == null || buffer.length != this.buffer.length || buffer[0].length != this.buffer[0].length) {
			this.buffer = new boolean[imheight][];
			for (int i = 0; i < imheight; i++)
				this.buffer[i] = new boolean[imwidth];
		}
		
		this.imwidth = imwidth;
		this.imheight = imheight;
		
		
		for (int y = 0; y < imheight; y++)
			for (int x = 0; x < imwidth; x++)
				this.buffer[y][x] = buffer[y][x];
		
		repaint();
	}
}
