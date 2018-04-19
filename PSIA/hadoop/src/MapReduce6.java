
import java.io.IOException;
import java.util.ArrayList;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;

public class MapReduce6 {

  public static class TokenizerMapper 
       extends Mapper<Object, Text, MyKey, Text>{
	  
	private Text word = new Text();
    private static final char[] ponctuation = {' ',',','?','!','.',':',';','\''};
    private static int compteurLignes = 0;
    private static Object synchro = new Object();
    
    public void map(Object key, Text value, Context context
                    ) throws IOException, InterruptedException {
    	synchronized(synchro){
    		compteurLignes++;
    	}
    	
    	String p = new String(ponctuation);
        StringTokenizer itr = new StringTokenizer(value.toString(), p, false);
        
        int position = 0;
        
        while (itr.hasMoreTokens()) {
        	
        	MyKey cle = new MyKey();
        	
        	cle.setLine(compteurLignes);
        	cle.setPosition(++position);
        	
        	word.set(itr.nextToken());
        	
        	context.write(cle, word);
        }
    }
  }
  
  public static class CharCountReducer 
       extends Reducer<MyKey,Text,MyKey,IntWritable> {

    public void reduce(MyKey key, Iterable<Text> values, 
                       Context context
                       ) throws IOException, InterruptedException {
    	
        for (Text val : values) {
        	context.write(null, new IntWritable(val.toString().length()));		
        }
      
    }
  }

  public static void main(String[] args) throws Exception {
	  
		Configuration conf = new Configuration();

		String[] otherArgs = new GenericOptionsParser(conf, args).getRemainingArgs();
		if (otherArgs.length < 2) {
			System.err.println("Usage: MapReduce6 <in1> <out1> <in2> <out2> <...> <...>");
			System.exit(2);
		}
		
		ArrayList<Job> jobs = new ArrayList<Job>();
		
        for(int i = 0 ; i < otherArgs.length / 2; i++){
		//Configuration du Job
        	jobs.add(new Job(conf, "Nombre de lettres"));

		
        	jobs.get(i).setMapOutputKeyClass(MyKey.class);//type cle de sortie des maps
        	jobs.get(i).setMapOutputValueClass(Text.class);//type valeur de sortie des maps
		
		
        	jobs.get(i).setOutputKeyClass(MyKey.class);//type cle de sortie du MapReduce
        	jobs.get(i).setOutputValueClass(IntWritable.class);//type valeur de sortie du MapReduce

        	jobs.get(i).setMapperClass(TokenizerMapper.class);
        	jobs.get(i).setReducerClass(CharCountReducer.class);
		
        	jobs.get(i).setNumReduceTasks(1);
        	jobs.get(i).setJarByClass(MapReduce6.class); 
		
		
        	FileInputFormat.addInputPath(jobs.get(i), new Path(otherArgs[i * 2]));
	    
        	final Path outDir = new Path(otherArgs[(i * 2) + 1]);
        	FileOutputFormat.setOutputPath(jobs.get(i), outDir);
        	final FileSystem fs = FileSystem.get(conf);//r��cup��ration d'une r��f��rence sur le syst��me de fichier HDFS
        	if (fs.exists(outDir)) {
        		fs.delete(outDir, true);
        	}
	   
        }
        
        for(int i = 0 ; i < jobs.size(); i++){
        	jobs.get(i).waitForCompletion(true);
        }
  }
}

