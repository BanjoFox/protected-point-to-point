/*
 * A class to test the JNI interface.
 */

public class p3secondaryAPI
{
 /**
  *  The p3secondaryAPI constructor.
  */
	public p3secondaryAPI()
	{
	}
/**
 *  The p3secondaryAPI constructor.
 */
	public static void main(String[] args)
	{
		p3secWindow secWindow = new p3secWindow(args);
		secWindow.setLocation(300, 200);
		secWindow.start();
	}
}
