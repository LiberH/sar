import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;

import org.apache.hadoop.io.WritableComparable;


public class MyKey implements WritableComparable<MyKey>{
	protected int line;
	protected int position;
	
	public MyKey(){
		this.line = -1;
		this.position = -1;
	}
	
	public MyKey(int line, int position){
		setLine(line);
		setPosition(position);
	}
	
	public int getLine() {
		return line;
	}

	public void setLine(int line) {
		this.line = line;
	}

	public int getPosition() {
		return position;
	}

	public void setPosition(int position) {
		this.position = position;
	}

	@Override
	public void readFields(DataInput arg0) throws IOException {
		// TODO Auto-generated method stub
		this.line = arg0.readInt();
		this.position = arg0.readInt();
		
	}

	@Override
	public void write(DataOutput arg0) throws IOException {
		// TODO Auto-generated method stub
		arg0.writeInt(this.line);
		arg0.writeInt(this.position);
		
	}

	@Override
	public int compareTo(MyKey o) {
		// TODO Auto-generated method stub
		if(((this.line < o.line) || (this.line == o.line && this.position < o.position)))
			return -1;
		else if(this.line == o.line && this.position == o.position)
			return 0;
		else
			return 1;
	}

	public boolean equals(Object o) {
		if (!(o instanceof MyKey)) {
			return false;
		}
		
		MyKey other = (MyKey) o;
		return ((this.line == other.line) && (this.position == other.position));
	}

}
