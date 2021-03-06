
import java.io.IOException;
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

public class MapReduce5 {

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
    	
    	String regex = new String(ponctuation);
        StringTokenizer itr = new StringTokenizer(value.toString(), regex, false);
        
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
		if (otherArgs.length != 2) {
			System.err.println("Usage: MapReduce5 <in> <out>");
			System.exit(2);
		}
		

		//Configuration du Job
		Job job = new Job(conf, "Nombre de lettres");

		
		job.setMapOutputKeyClass(MyKey.class);//type cle de sortie des maps
		job.setMapOutputValueClass(Text.class);//type valeur de sortie des maps
		
		
		job.setOutputKeyClass(MyKey.class);//type cle de sortie du MapReduce
		job.setOutputValueClass(IntWritable.class);//type valeur de sortie du MapReduce

		job.setMapperClass(TokenizerMapper.class);
		job.setReducerClass(CharCountReducer.class);
		
		job.setNumReduceTasks(1);
		job.setJarByClass(MapReduce5.class); 
		
		
	    FileInputFormat.addInputPath(job, new Path(otherArgs[0]));
	    final Path outDir = new Path(otherArgs[1]);
	    FileOutputFormat.setOutputPath(job, outDir);
	    final FileSystem fs = FileSystem.get(conf);
		 if (fs.exists(outDir)) {
			 fs.delete(outDir, true);
		 }
	   
	    System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}
