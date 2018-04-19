package risksim;

import java.util.Random;

import peersim.config.*;
import peersim.core.*;

public class Initializer implements Control {

	private String prefix;
	private int tpid;
	private int rpid;
	private int redpct;
	
	public Initializer(String prefix) {
		this.prefix = prefix;
		this.tpid = Configuration.getPid(this.prefix + ".tpid");
		this.rpid = Configuration.getPid(this.prefix + ".rpid");
		this.redpct = Configuration.getInt(this.prefix + ".redpct");
	}
	
	@Override
	public boolean execute() {
		int networkSize = Network.size();
		Random rand = new Random();
		for (int nodeID = 0; nodeID < networkSize; nodeID++) {
			Node node = Network.get(nodeID);
			TransportProtocol tp = (TransportProtocol)node.getProtocol(this.tpid);
			RiskProtocol rp = (RiskProtocol)node.getProtocol(this.rpid);
			
			rp.setId(nodeID);
			rp.setTransportProtocol(tp);
			rp.setColor((rand.nextInt(100) < this.redpct) ? RiskProtocol.RED : RiskProtocol.BLUE);
			rp.send(node, new Message(Message.WAKEUP, 0), false);
			
			// Neighborhood
			int neighborID;
			int neighborNumber = 8 + rand.nextInt(7);
			for (int i = rp.getNeighborList().size(); i < neighborNumber; i++) {
				Node neighbor;
				RiskProtocol rpn;
				do {
					neighborID = rand.nextInt(networkSize);
					neighbor = Network.get(neighborID);
					rpn = (RiskProtocol)neighbor.getProtocol(this.rpid);
				} while (rp.getNeighborList().contains(neighbor) ||
						neighborID == nodeID ||
						rpn.getNeighborList().size() >= 15);
				
				rp.addNeighbor(neighbor);
				rpn.addNeighbor(node);
			}
		}
		
		return false;
	}
}
