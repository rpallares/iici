


package entrepot;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import entrepot.OlapArbre.NomExistantException;

/* classe decrivant une requete olap, permet d'optimiser cette derniere et de la transformer */
public class OlapRequete {
	
	public class MalformedRequestException extends Exception {
		private static final long serialVersionUID = 1L;
		public MalformedRequestException() { super (); }
	}
	
	public enum Operator { NULL("NULL",false),
		COUNT("COUNT", false), SUM("SUM", true), AVG("AVG", true), MIN("MIN", true), MAX("MAX", true);
		
		private String valeur;
		private boolean isNum;
		private Operator(String s, boolean b) {
			valeur = s;
			isNum = b;
		}
		public String getValeur() {
			return valeur;
		}
		public boolean isNum() {
			return isNum;
		}
		
		static public Operator[] getListeOperateur(OlapArbre a) {
			Operator[] tOp = OlapRequete.Operator.values();
			if(a == null) return tOp;
			SQLType t = a.getType();
			ArrayList<Operator> res = new ArrayList<Operator>();
			
			for(int i=0; i<tOp.length; i++) {
				if(tOp[i].isNum() == t.isNumeric()) {
					res.add(tOp[i]);
				}
			}
			if(!res.contains(Operator.COUNT)) {
				res.add(Operator.COUNT);
			}
			Operator[] r=new Operator[res.size()];
			res.toArray(r);
			return r;
		}
	}


	private List<Integer> ch;
	
	private Integer quantificateur;
	//private List<Operator> operateurs;
	private Operator operateur;
	
	//restriction sur la requete
	private List<Contrainte>restrictions;
	
	
	
	
	/* Constructeur de la classe */
	public OlapRequete (List<Integer> c, Integer q, Operator o, List<Contrainte>rest) {
		ch = new ArrayList<Integer>(c);
		quantificateur = q;
		operateur = o;
		if(!rest.isEmpty()){
			restrictions = new ArrayList<Contrainte>(rest);
		}
		else{
			restrictions = new ArrayList<Contrainte>();
		}
		
	}
	
	/**
	 * Retourne la chaine de caractere contenant la requete sql corespondante
	 * Seul le premier opÃ©rateur est utilisÃ©
	 * un seul quantificateur autorisÃ© ==> chemin peu etre mieu
	 * @param a arbre olap sur lequel va etre effectuÃ© la requete
	 * @return la chaine avec la requete sql
	 * @throws MalformedRequestException lorsque aucun parametre n'a ete precisÃ©
	 */
	public String sqla(OlapArbre o) throws MalformedRequestException {
		
		String args = "";
		String group = "";
		String groupRest = " ";
		int rest = 0;
		
		int id;
		// les tables  explorer ds la requetes
		List<String> tables = new ArrayList<String>();
		tables.add("TFO");
		//String nom = "TFO";
		String where = "WHERE ";
		int nb = 0;
		
		//Pour chaque attribut de projection
		for(int i = 0; i < ch.size(); i++){ 
			String nom = "TFO";
			// l'id de l'attribut
			id = ch.get(i);
			//l'arbre correspondant  l'attribut
			OlapArbre p = getArbre(o,id);
			System.out.println(p.getId()+" : "+p.getNom());
			
			//Vrifier si c'est un attribut de base
			if ((p.getPere()== p.getRoot()) || (p.getPere() == null )){
				args = args+nom+"."+OlapArbre.hashTable.get(id).getNom();
				//Ajouter la table de faits  la liste des tables  joindre
				//if(!tables.contains("TFO")) tables.add("TFO");
				//System.out.println("TFO");
			}
			else{ // sinon chercher l'attribut de base qui lui est li
				//p.getPereBase1();
				nom = "TO"+p.getPereBase1().getNom();
				if(!tables.contains(nom)){
					if(nb>0) where = where+" AND ";
					where = where+" "+"TFO"+"."+p.getPereBase1().getNom()+"="+nom+"."+p.getPereBase1().getNom();
					nb = nb +1;
					tables.add(nom);
					
				}
				args = args+nom+"."+OlapArbre.hashTable.get(id).getNom();
				
			}
			//construire la liste des attribut de projection de la requete (SELECT)
			//args = args+nom+"."+OlapArbre.hashTable.get(id).getNom();
			if(i<= ch.size()-2){
				args = args+",";
			}
		}
		
		
		
		
		//l'arbre correspondant  l'attribut
		OlapArbre q = getArbre(o,quantificateur);
		System.out.println(q.getId()+" : "+q.getNom());
		
		//Vrifier si c'est un attribut de base
		if ((q.getPere()== q.getRoot()) || (q.getPere() == null )){
			
		}
		else{ // sinon chercher l'attribut de base qui lui est li
			//p.getPereBase1();
			if(nb>0) where = where+" AND ";
			String qu = "TO"+q.getPereBase1().getNom();
			where = where+" "+"TFO"+"."+q.getPereBase1().getNom()+"="+qu+"."+q.getPereBase1().getNom();
			if(!tables.contains(qu)) {
				tables.add(qu);
			}
			
			
		}
		
		String s = "";
		String cont = "";
		String req = "SELECT ";
			s += ","+operateur+"("+OlapArbre.hashTable.get(quantificateur).getNom()+") as TQy ";
		
		
		
		
		
		

				//S'il y'a des restrictions
		if(restrictions.isEmpty()){
			System.out.println("pas de contraintes");
			//req = req+args+""+s+" "+"FROM "+table+" "+group;
		}
		else{
			System.out.println("avec contraintes");
			
			int nbRestrictions = 0;
			//Pour chaque tuple de restriction
			for(int w = 0; w < restrictions.size();w++){
				//operande, operateur et valeur de la restriction
				int opd =  restrictions.get(w).getOperande();
				String op = restrictions.get(w).getOperator();
				String val = restrictions.get(w).getValeur();
				// si l'operande est un attribut de table
				if(hasAttribut(opd)){
						
						
						//if(a == opd){
							//OlapArbre ab = getArbre(o,i);
					         OlapArbre ab = OlapArbre.hashTable.get(opd);
					         
							if(ab.getType().equals(SQLType.VARCHAR)  ){
								if(op.equals("=")){// = est le reul oprateur autoris pour les VARCHAR
									
									String t = "TO"+ab.getPereBase1().getNom();
									//String qu = "TO"+q.getPereBase1().getNom();
									//where = where+" "+"TFO"+"."+q.getPereBase1().getNom()+"="+qu+"."+q.getPereBase1().getNom();
									if(!tables.contains(t)) {
										tables.add(t);
									}
									
									//if(nbRestrictions == 0) cont = "WHERE ";
								    //if(nbRestrictions>0) cont = cont+" AND ";
									if(nb>0) where = where+" AND ";
									
								    val = "'"+val+"'";//Entourer la valeur par des cotes
								    where = where+'('+t+"."+ab.getNom()+op+val+')';
									nbRestrictions ++;
								    System.out.println(ab.getType());
								    //System.out.println("SELECT * FROM clients WHERE mail='"+opd+"' and pwd='"+val+"'");
									
								} 
									
								
							}
							else{
								if(nb>0) where = where+" AND ";
								//else cont = "WHERE ";
								where = where+"("+ab.getNom()+op+val+")";
								nbRestrictions ++;
								System.out.println(ab.getType());
								
							}
						//}
						
					//}
					System.out.println("C bon");
					
					
					
				}
					
				
				
			}
			//req = req+args+""+s+" FROM "+table+" "+cont+" "+group;
			 
		}
		
		//Liste des tables sur lesquelles on va appliquer la jointure
		String table = "";
		if(tables.size()==1){
			table = table+tables.get(0);
			group = " group by "+","+args;
			req = req+args+""+s+" "+"FROM "+table+" "+group;
		} 
		else{
			//table = table+" join(";
			for(int n = 0; n< tables.size();n++){
				table = table+tables.get(n);
				if(n<= tables.size()-2){
					table = table+",";
				}
			}
			table = table+" "+where+" ";
			group = " group by "+args;
			req = req+args+""+s+" FROM "+table+" "+group;
		}
		
		return req;
	
	}
	
	/* transforme et optimise la requete olap, peu etre a faire dans le sql??? */
	private OlapArbre getArbre(Object parent, int id) {
		OlapArbre a = (OlapArbre) parent;
		List <OlapArbre> fils = new ArrayList<OlapArbre>(a.getFils());
		for(int i =0;i< fils.size();i++){
			if(fils.get(i).getId()==id) return fils.get(i);
			else{
				OlapArbre p = getArbre(fils.get(i), id);
				if(p!=null){
					return p;
				}
			}
		}
		return null;
	}
	
	//Verifier si l'attribut sur lequel on veut appliquer une restriction existe ds les tables
	private boolean hasAttribut(int att){
		return OlapArbre.hashTable.containsKey(att);
	}
	public static void main(String[] args) throws MalformedRequestException {
		/*BDRelationelle ma_bd = new BDRelationelle();
		ma_bd.connect();
		
		String req = "CREATE TABLE customer (First_Name char(50),Last_Name char(50),Address char(50) default 'Unknown',"
			+"City char(50) default 'Mumbai',Country char(25),Birth_Date date);";
		int b = ma_bd.executeUpdate(req);
		System.out.println(b);
		
		req = "select * from customer;";
		ResultSet rs = ma_bd.executeQuery(req);
		System.out.println(rs.toString());
		System.out.println("fin");
		ma_bd.unconnect();*/
		
		try {
			
			//Pour les tables que j'ai cres j'avais besoin de sparer la definition de chaque noeuds
			//Cela n'a aucune consquence sur le code que tu as fait jusque l
			//Mais c'tait oblig
			OlapArbre a = new OlapArbre("O", SQLType.VARCHAR);
			
			OlapArbre Year = new OlapArbre("Year", SQLType.INTEGER);
			OlapArbre Month = new OlapArbre("Month", SQLType.INTEGER,Year);
			OlapArbre date = new OlapArbre("Date", SQLType.DATE, Month);
			
			OlapArbre Region = new OlapArbre("Region", SQLType.VARCHAR);
			OlapArbre City =new OlapArbre("City", SQLType.VARCHAR,Region);
			OlapArbre store = new OlapArbre("Store", SQLType.VARCHAR,City);
			
			OlapArbre Category = new OlapArbre("Category", SQLType.VARCHAR);
			OlapArbre Supplier = new OlapArbre("Supplier", SQLType.VARCHAR);
			OlapArbre product = new OlapArbre("Product", SQLType.VARCHAR);
			product.ajouterFils(Category);
			product.ajouterFils(Supplier);
			
			OlapArbre q = new OlapArbre("Quantity", SQLType.INTEGER);
			
			//Je les ajoute dans ma table noeud 1 par 1
			
			
			//Ma table Bases va contenir tous les noeuds lis au noeuds initial
			//Me facile le parcour et l'indexation
			
			a.ajouterFils(date);
			a.ajouterFils(store);
			a.ajouterFils(product);
			a.ajouterFils(q);
			
			String req = a.toSQLStarSchema();
			
			System.out.println(req);
			List<Integer> c = new ArrayList<Integer>();
			 c.add(1);
			  c.add(2);
			  c.add(3);
			  c.add(4);
			 Integer qu = 9;
			 List <Contrainte> rest = new ArrayList<Contrainte>();
			 Contrainte cont = new Contrainte(6,"<","Paris");
			 rest.add(cont);
			 Contrainte cont1 = new Contrainte(10,"=","Paris");
			 rest.add(cont1);
				//OlapRequete ol = new OlapRequete(c,q,Operator.SUM); 
			    OlapRequete ol = new OlapRequete(c,qu, Operator.SUM,rest);
				String res = ol.sqla(a);
				System.out.println(res);
			
			/*BDRelationelle ma_bd = new BDRelationelle();
			ma_bd.connect();
			ma_bd.executeUpdate(req);
			ma_bd.unconnect();*/
			
		} catch (NomExistantException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}
}