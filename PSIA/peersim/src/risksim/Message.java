package risksim;

public class Message {
	
	public static final int WAKEUP = 0;
	public static final int ATTACK = 1;
	public static final int DEFEND = 2;
	
	private int tag;
	private int value;
	
	public Message(int tag, int value) {
		this.tag = tag;
		this.value = value;
	}
	
	@Override
	public String toString() {
		String str = new String();
		switch (this.tag) {
		case Message.WAKEUP:
			str += "WAKEUP";
			break;
		case Message.DEFEND:
			str += "DEFEND ";
			str += this.value;
			break;
		case Message.ATTACK:
			str += "ATTACK ";
			str += this.value;
			break;
		}
		return str;
	}
	
	public int getTag() {
		return this.tag;
	}
	
	public int getValue() {
		return value;
	}
}
