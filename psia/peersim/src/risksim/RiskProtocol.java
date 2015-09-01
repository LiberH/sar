package risksim;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.Random;

import peersim.config.*;
import peersim.core.*;
import peersim.edsim.*;

public class RiskProtocol implements EDProtocol {

	public static final int BLUE =  0;
	public static final int RED  =  1;
	
	private String prefix;
	private int id;
	private int pid;
	private int tpid;
	private int color;
	private int soldiers;
	private int redsolpct;
	private int bluesolpct;
	private int attpct;
	private TransportProtocol transportProtocol;
	private ArrayList<Node> neighborList;
	
	public RiskProtocol(String prefix) {
		this.prefix = prefix;
		this.pid  = Configuration.getPid(this.prefix + ".pid");
		this.tpid = Configuration.getPid(this.prefix + ".tpid");
		this.soldiers = Configuration.getInt(this.prefix + ".soldiers");
		this.redsolpct = Configuration.getInt(this.prefix + ".redsolpct");
		this.bluesolpct = Configuration.getInt(this.prefix + ".bluesolpct");
		this.attpct = Configuration.getInt(this.prefix + ".attpct");
		this.neighborList = new ArrayList<Node>();
	}
	
	public void attack() {
		double attackers;
		Iterator<Node> iterator;
		Random rand = new Random();
		Node receiver, node;
		RiskProtocol app;
		Message message;
		
		// Find an enemy neighbor node
		Collections.shuffle(this.neighborList);
		iterator = this.neighborList.iterator();
		receiver = null;
		while (iterator.hasNext()) {
			node = (Node)iterator.next();
			app = (RiskProtocol)node.getProtocol(this.pid);
			if (app.getColor() != this.color) {
				receiver = node;
				break;
			}
		}

		// No enemy
		if (receiver == null) return;
		
		attackers = (double)this.soldiers * ((double) ((this.color == RiskProtocol.RED) ? this.redsolpct : this.bluesolpct)/100.0);
		this.soldiers -= (int)attackers;
		
		message = new Message(Message.ATTACK, (int)attackers);
		this.send(receiver, message, true);
	}
	
	public void defend() {
		double defenders;
		Iterator<Node> iterator;
		Random rand = new Random();
		Node receiver, node;
		RiskProtocol app;
		Message message;
		
		// Find an ally neighbor node
		Collections.shuffle(this.neighborList);
		iterator = this.neighborList.iterator();
		receiver = null;
		while (iterator.hasNext()) {
			node = (Node)iterator.next();
			app = (RiskProtocol)node.getProtocol(this.pid);
			if (app.getColor() == this.color){
				receiver = node;
				break;
			}
		}

		// No ally
		if (receiver == null) return;
		
		defenders = (double)this.soldiers * ((double) ((this.color == RiskProtocol.RED) ? this.redsolpct : this.bluesolpct)/100.0);
		//System.out.println("*" + (int)defenders);
		this.soldiers -= (int)defenders;
		message = new Message(Message.DEFEND, (int)defenders);
		this.send(receiver, message, true);
	}
	
	@Override
	public void processEvent(Node node, int pid, Object event) {
		Random rand = new Random();
		Message message = (Message)event;
		switch (message.getTag()) {
		case Message.WAKEUP:
			if (rand.nextInt(100) < this.attpct) this.attack();
			else this.defend();
			
			this.send(node, new Message(Message.WAKEUP, 0), false);
			break;
			
		case Message.ATTACK:
			// Planet successfully defended
			int attackers = message.getValue();	
			if (this.soldiers < attackers)
				this.color = (this.color == RiskProtocol.BLUE) ? RiskProtocol.RED : RiskProtocol.BLUE;
			this.soldiers += attackers;
			break;
			
		case Message.DEFEND:
			// Update planet's soldier number
			int defenders = message.getValue();	
			this.soldiers += defenders;
			break;
		}
	}
	
	public void send(Node node, Message message, boolean immediate) {
		this.transportProtocol.send(node, message, immediate, this.color == RiskProtocol.RED);
	}
	
	public void addNeighbor(Node node) {
		this.neighborList.add(node);
	}
	
	@Override
	public Object clone() {
		return new RiskProtocol(this.prefix);
	}
	
	public void setId(int id) {
		this.id = id;
	}
	
	public void setTransportProtocol(TransportProtocol transportProtocol) {
		this.transportProtocol = transportProtocol;
	}
	
	public void setColor(int color) {
		this.color = color;
	}
	
	public int getId() {
		return id;
	}
	
	public int getColor() {
		return color;
	}
	
	public ArrayList<Node> getNeighborList() {
		return this.neighborList;
	}
}
