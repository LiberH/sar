import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;


public class MyKeyMix extends MyKey{
	private int id;
	
	public MyKeyMix(){
		super();
		this.id = -1;
	}
	
	public MyKeyMix(int line, int position){
		super(line, position);
		setId(id);
	}
	
	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	@Override
	public void readFields(DataInput arg0) throws IOException {
		// TODO Auto-generated method stub
		super.readFields(arg0);
		this.id = arg0.readInt();
		
	}

	@Override
	public void write(DataOutput arg0) throws IOException {
		// TODO Auto-generated method stub
		super.write(arg0);	
		arg0.writeInt(this.id);
		
	}
	
	@Override
	public int compareTo(MyKey o) {
		// TODO Auto-generated method stub
		if(this.id == ((MyKeyMix)o).id){
			return super.compareTo(o);
		}
		
		return this.id - ((MyKeyMix)o).id;
	}

	public boolean equals(Object o) {
		if (!(o instanceof MyKeyMix)) {
			return false;
		}
		
		MyKeyMix other = (MyKeyMix) o;
		return (super.equals(o) == true && (this.id == other.id));
	}

}