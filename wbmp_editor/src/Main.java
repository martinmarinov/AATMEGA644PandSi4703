import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.imageio.ImageIO;
import javax.swing.*;

public class Main extends JPanel implements Drawer.OnChangedCallback {

	private static final long serialVersionUID = 8556617938392122525L;
	private JButton load;
    private JButton export;
    private JButton fromimage;
    private JTextArea output;
    private JTextField width;
    private JLabel jcomp5;
    private JTextField height;
    private JTextField halftones;
    private JLabel halftonesdesc;
    private JButton prepare;
    private Drawer drawer;
    private Preview preview;
    final private JFileChooser fc = new JFileChooser();

    public Main() {
        //construct components
        load = new JButton ("From Clip");
        export = new JButton ("To Clip");
        output = new JTextArea (5, 5);
        width = new JTextField ("84", 5);
        jcomp5 = new JLabel ("x");
        halftonesdesc = new JLabel("Halftones:");
        halftones = new JTextField ("4", 5);
        height = new JTextField ("48", 5);
        prepare = new JButton ("Prepare");
        drawer = new Drawer();
        preview = new Preview();
        fromimage = new JButton ("From Img");

        //adjust size and set layout
        setPreferredSize (new Dimension (911, 686));
        setLayout (null);

        //add components
        add (load);
        add (export);
        add (output);
        add (width);
        add (jcomp5);
        add (height);
        add (prepare);
        add (drawer);
        add (preview);
        add (fromimage);
        add (halftones);
        add (halftonesdesc);
        
        drawer.registerCallback(preview);
        drawer.registerCallback(this);
        
        prepare.addActionListener(new ActionListener() {
			
			@Override
			public void actionPerformed(ActionEvent arg0) {
				final int proposedheight = Integer.valueOf(height.getText());
				height.setText(String.valueOf((proposedheight/8)*8));
				
				drawer.setSize(Integer.valueOf(width.getText()), Integer.valueOf(height.getText()));
			}
		});
        prepare.doClick();
        
        export.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				output.setText(drawer.getCCode());
				output.selectAll();
				output.copy();
			}
		});

        load.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				try {
					output.selectAll();
					output.paste();
					drawer.fromCCode(output.getText());
				} catch (Throwable e) {
					onError(e);
				}
			}
		});
        
        
        fromimage.addActionListener(new ActionListener() {
        	public void actionPerformed(ActionEvent arg0) {
        		if (fc.showOpenDialog(Main.this) == JFileChooser.APPROVE_OPTION) {
        			try {
        				drawer.fromPicture(ImageIO.read(fc.getSelectedFile()), Integer.valueOf(halftones.getText()));
        			} catch (Throwable e) {
        				onError(e);
        			}
        		}
        	}
        });

        //set component bounds (only needed by Absolute Positioning)
        load.setBounds (0, 495, 100, 20);
        export.setBounds (100, 495, 100, 20);
        output.setBounds (0, 515, 915, 175);
        width.setBounds (705, 0, 100, 25);
        jcomp5.setBounds (805, 0, 15, 25);
        height.setBounds (815, 0, 100, 25);
        prepare.setBounds (755, 30, 100, 25);
        preview.setBounds (705, 335, 205, 160);
        drawer.setBounds (0, 0, 705, 495);
        fromimage.setBounds (500, 495, 100, 20);
        halftonesdesc.setBounds (600, 492, 100, 25);
        halftones.setBounds (700, 492, 100, 25);
        
    }
    
    private void onError(final Throwable t) {
    	t.printStackTrace();
    	JOptionPane.showMessageDialog(null, t.getLocalizedMessage(), t.getClass().getSimpleName(), JOptionPane.ERROR_MESSAGE);
    }


    public static void main (String[] args) {
        JFrame frame = new JFrame ("Black And White Painter");
        frame.setDefaultCloseOperation (JFrame.EXIT_ON_CLOSE);
        frame.getContentPane().add (new Main());
        frame.pack();
        frame.setVisible (true);
    }


	@Override
	public void onChanged(boolean[][] buffer, int imwidth, int imheight) {
		try {
			if (Integer.valueOf(height.getText()) != imheight) height.setText(String.valueOf(imheight));
			if (Integer.valueOf(width.getText()) != imwidth) width.setText(String.valueOf(imwidth));
		} catch (Throwable e) {
			onError(e);
		}
	}
}
