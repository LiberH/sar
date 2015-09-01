package risksim;

import peersim.config.*;
import peersim.core.*;
import peersim.edsim.*;

public class TransportProtocol implements Protocol {
	
	private String prefix;
	private int pid;
	private int rpid;
	private int redmin;
	private int redrange;
	private int bluemin;
	private int bluerange;
	
	public TransportProtocol(String prefix) {
		this.prefix = prefix;
		this.pid   = Configuration.getPid(this.prefix + ".pid");
		this.rpid  = Configuration.getPid(this.prefix + ".rpid");
		this.redmin   = Configuration.getInt(this.prefix + ".redmin");
		this.redrange = Configuration.getInt(this.prefix + ".redmax") - this.redmin;

		this.bluemin   = Configuration.getInt(this.prefix + ".bluemin");
		this.bluerange = Configuration.getInt(this.prefix + ".bluemax") - this.bluemin;
	}
	
	public void send(Node receiver, Message message, boolean immediate, boolean isred) {
		long latency = 0;
		if (!immediate)
			latency = this.randomLatency(isred);
		
    	EDSimulator.add(latency, message, receiver, this.rpid);
    }

    public long randomLatency(boolean isred) {
    	long latency = (isred) ? this.redmin : this.bluemin;
    	if (((isred) ? this.redrange : this.bluemin) > 0)
    		latency += CommonState.r.nextLong(((isred) ? this.redrange : this.bluemin));
    	return latency;
    }
    
	@Override
	public Object clone() {
		return this;
	}
}
