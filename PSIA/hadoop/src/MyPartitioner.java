import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Partitioner;


public class MyPartitioner extends Partitioner<MyKeyMix, Text> {

	@Override
	public int getPartition(MyKeyMix arg0, Text arg1, int nbreReduces) {
		
		if(nbreReduces == 0)
			return 0;
		
		//Retourne la partition 0 si Poeme est miniscule sinon partition 1
		if(arg0.getId() == 0)
			return 0;
		else 
			return 1;
	}

}
