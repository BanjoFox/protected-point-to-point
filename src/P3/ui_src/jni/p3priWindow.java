/*
 * P3 API window handler
 */

import java.awt.Container;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.JTextArea;

public class p3priWindow extends JFrame
{
	static final long serialVersionUID = 1L;

	private p3priAPI priAPI = null;
	private Container jtWindow = null;
	private GridBagLayout gbLayout = null;
	private JTextField txRequest = null;
	private JTextArea txResponse = null;
	private JButton bSend = null;
	private JButton bNext = null;
	private JButton bExit = null;
 
 /**
  *  The P3 primary window constructor.
  */
	public p3priWindow(String[] args)
	{
		int i;
		String response;

		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setTitle("P3 Primary");
		jtWindow = getContentPane();
		gbLayout = new GridBagLayout();
		jtWindow.setLayout(gbLayout);
		initLayout();
		priAPI = new p3priAPI();
		try {
			priAPI.initAdmin(args);
		} catch (IllegalArgumentException err) {
			System.out.println("Init error\n" + err);
		}
	}

/**
 * Open the Convertator Window
 */
	public void start() {
		pack();
		setVisible(true);
	}

/**
 * Initialize the main Convertator window.
 */
	private void initLayout() {
		JLabel label;
		GridBagConstraints c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;

	    // Request [ text ]
		// Response [ text ]
		// (Send) (Next) (Exit)
		c.insets = new Insets(2,3,2,3);
		label = new JLabel("Request");
		c.weightx = 0.1; c.gridwidth = 1; c.gridx = 0; c.gridy = 0;
		gbLayout.setConstraints(label, c); jtWindow.add(label);
		txRequest = new JTextField(40);
		txRequest.setEditable(true);
		c.weightx = 0.9; c.gridwidth = 4; c.gridx = 1; c.gridy = 0;
		gbLayout.setConstraints(txRequest, c); jtWindow.add(txRequest);

		label = new JLabel("Response");
		c.weightx = 0.1; c.gridwidth = 1; c.gridx = 0; c.gridy = 1;
		gbLayout.setConstraints(label, c); jtWindow.add(label);
		txResponse = new JTextArea(3, 40);
		txResponse.setEditable(false);
		c.weightx = 0.9; c.gridwidth = 4; c.gridheight = 3;
		c.gridx = 1; c.gridy = 1;
		gbLayout.setConstraints(txResponse, c); jtWindow.add(txResponse);

		c.anchor = GridBagConstraints.LINE_START;
		bSend = new JButton("Send");
		c.weightx = 0.3; c.gridwidth = 1; c.gridx = 1; c.gridy = 4;
		gbLayout.setConstraints(bSend, c); jtWindow.add(bSend);
		bSend.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) { sendRequest(); }
		});
		bNext = new JButton("Next");
		c.weightx = 0.3; c.gridwidth = 1; c.gridx = 2; c.gridy = 4;
		gbLayout.setConstraints(bNext, c); jtWindow.add(bNext);
		bNext.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) { getNext(); }
		});
		bExit = new JButton("Exit");
		c.weightx = 0.3; c.gridwidth = 1; c.gridx = 3; c.gridy = 4;
		gbLayout.setConstraints(bExit, c); jtWindow.add(bExit);
		bExit.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) { System.exit(0); }
		});
	}

/*
 * Call the library to get a message.
 */
	private void sendRequest() {
		String response;

		//Call native method to load p3priAPI.java
		response = priAPI.getResponse(txRequest.getText().toString());
		txResponse.setText(response);
	}

/*
 * Get the next message.
 */
	private void getNext() {
		String response;

		response = priAPI.getNext(txRequest.getText().toString());
		txResponse.setText(response);
	}

}

