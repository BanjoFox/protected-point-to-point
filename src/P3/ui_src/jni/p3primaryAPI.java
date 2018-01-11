/*
 * A class to test the JNI interface.
 */

public class p3primaryAPI
{
 /**
  *  The p3primaryAPI constructor.
  */
	public p3primaryAPI()
	{
	}
/**
 *  The p3primaryAPI constructor.
 */
	public static void main(String[] args)
	{
		p3priWindow priWindow = new p3priWindow(args);
		priWindow.setLocation(300, 200);
		priWindow.start();
	}
}
