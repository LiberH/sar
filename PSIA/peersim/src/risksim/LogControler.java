package risksim;

import java.io.IOException;

import peersim.config.*;
import peersim.core.*;

public class LogControler implements Control {

	private String prefix;
	private int rpid;
	
	public LogControler(String prefix) {
		this.prefix = prefix;
		this.rpid = Configuration.getPid(this.prefix + ".rpid");
	}
	
	@Override
	public boolean execute() {
		int reds  = 0;
		int blues = 0;
		int networkSize = Network.size();
		for (int nodeID = 0; nodeID < networkSize; nodeID++) {
			Node node = Network.get(nodeID);

			RiskProtocol rp = (RiskProtocol)node.getProtocol(this.rpid);
			reds  += (rp.getColor() == RiskProtocol.RED ) ? 1 : 0;
			blues += (rp.getColor() == RiskProtocol.BLUE) ? 1 : 0;
		}
		
		try {
			LogFile.init("reds.csv");
			LogFile.write(reds + ",");
		} catch (IOException exception) {
			System.out.println(exception.getMessage());
			System.exit(1);
		}

		return false;
	}
}
