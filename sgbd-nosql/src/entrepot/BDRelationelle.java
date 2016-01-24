package entrepot;

import java.io.*;
import java.sql.*;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import entrepot.OlapArbre.NomExistantException;

//beaucoup de sql exception... a gérer
//sinon c une ptite base a tester et améliorer
public class BDRelationelle {

    private String url;
    Connection connection;
    DatabaseMetaData dbmd;
    String pilote;

    /* nom de la base mysql : edd_database;
     * nom utilisateur : edd_program
     * pass utilisateur : edd_password
     */
    public BDRelationelle(String _url, String _pilote) {
        url = _url;
        connection = null;
        dbmd = null;
        pilote = _pilote;
        try {
            Class.forName(_pilote);
        } catch (ClassNotFoundException e) {
            System.out.println("Erreur lors du chargement du pilote " + e.getMessage());
        }
    }

    /* connection avec mysql pour "edd_database"*/
    public BDRelationelle() {
        //this("jdbc:mysql://91.121.203.87/edd_database", "com.mysql.jdbc.Driver");
        this("jdbc:mysql://localhost:8889/edd_database", "com.mysql.jdbc.Driver");
    }

    public void connect(String login, String password) throws SQLException {
        Properties info = new Properties();
        info.setProperty("user", login);
        info.setProperty("password", password);
        connection = DriverManager.getConnection(url, info);
        dbmd = connection.getMetaData();
    }

    public void connect() {
        try {
            //this.connect("edd", "edd");
            this.connect("edd_program", "edd_password");
        } catch (SQLException e) {
            System.out.println("Impossible de se connecter a la base " + e.getMessage());
        }
    }

    public void unconnect() {
        try {
            connection.close();
        } catch (SQLException e) {
            System.out.println("La connection est deja fermée " + e.getMessage());
        }
    }

    public boolean is_connected() {
        return true;
    }

    public DatabaseMetaData getDBMetaData() {
        return dbmd;
    }

    /**
     * Execute toutes les requetes de type SELECT et retourne le resultSet
     * @param requete sql
     * @return resultSet de la requete
     */
    public ResultSet executeQuery(String requete) {
        Statement stmt;
        try {
            stmt = connection.createStatement();
            //verification de la validité de la requete????
            ResultSet rs = stmt.executeQuery(requete);
            return rs;
        } catch (SQLException e) {
            System.out.println("Impossible d'executer la requete " + e.getMessage());
            return null;
        }
    }

    /**
     * Permet d'executer toutes les requetes qui n'attendent pas
     * de résultats
     * CREATE ALTER DROP INSERT UPDATE DELETE GRANT REVOKE COMMIT ROLLBACK
     * @param requete sql
     * @return rien de tres interressant
     */
    public int executeUpdate(String req) {
        Statement stmt;
        String[] tab = req.split("___");
        try {
            stmt = connection.createStatement();
            for (String s : tab) {
                try {
                    stmt.executeUpdate(s);
                } catch (SQLException e) {
                    System.out.println("Impossible d'executer la requete : " + s + "\n" + e.getMessage());
                }
            }
            return 1;
        } catch (SQLException e) {
            System.out.println("Impossible d'executer l'update " + e.getMessage());
            return 0;
        }
    }

    /**
     * retourne l'arbre correspondant a la base de donnée en étoile
     * Attention Nécessite d'ajouter un treeListener si on veu l'utiliser graphiquement
     * @return l'arbre correspondant, null si impossible de créer l'arbre (schéma non étoile)
     * @throws SQLException
     * @throws NomExistantException 
     */
    public OlapArbre toOlapArbre() throws SQLException, NomExistantException {
        boolean connected = this.is_connected();

        if (!connected) {
            this.connect();
        }
        /*
        String req = "SELECT arbre from olapArbre;";
        ResultSet res = this.executeQuery(req);
        res.next();
        String s = res.getString("arbre");
        //System.out.println(s);
        OlapArbre a = null;
        try {
            a = (OlapArbre) OlapArbre.fromB64String(s);
            a.initHashTable();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }

        if (!connected) {
            this.unconnect();
        }*/
        String req = "SELECT src, type, dst, typef FROM olapArbre;";
        ResultSet res = this.executeQuery(req);
        Map<String, List<OlapArbre>> map = new Hashtable<String, List<OlapArbre>>();
        while(res.next()) {
        	String nom = res.getString("src");
        	SQLType t = SQLType.getSQLType(res.getInt("type"));
        	String f = res.getString("dst");
        	SQLType t2 = SQLType.getSQLType(res.getInt("typef"));
        	if(map.containsKey(nom)) {// si pere existe
        		boolean contains = false;
        		for(OlapArbre a : map.get(nom)) {
        			if(a.getNom() == f) contains = true;
        		}
        		if(!contains) { // si fils nest pas deja dans liste pere
        			OlapArbre a = map.get(nom).get(0);
        			OlapArbre fils = null;
        			if(map.containsKey(f)) { // si fils existe
        				List<OlapArbre> l = map.get(f);
        				fils = l.get(0);
        				a.ajouterFils(fils);
        				map.get(nom).add(fils);
        			}
        			else { //si fils existe pas
        				fils = new OlapArbre(f, t2);
        				a.ajouterFils(fils);
        				ArrayList<OlapArbre> l = new ArrayList<OlapArbre>();
        				l.add(fils);
        				map.put(f, l);
        				map.get(nom).add(fils);
        			}
        		}
        		else { System.out.println("Bizarre le fils a deja été créé");}
        	}
        	else {// si pere existe pas
        		OlapArbre a = new OlapArbre(nom, t);
        		OlapArbre fils = null;
        		if(map.containsKey(f)) {//si fils existe
        			fils = map.get(f).get(0);
        			a.ajouterFils(fils);
        		}
        		else {//si fils existe pas
        			fils = new OlapArbre(f, t2);
        			ArrayList<OlapArbre> l = new ArrayList<OlapArbre>();
        			l.add(fils);
        			a.ajouterFils(fils);
        		}
        		ArrayList<OlapArbre> l = new ArrayList<OlapArbre>();
        		l.add(a);
        		l.add(fils);
        		map.put(nom, l);
        	}
        }
        if (map.isEmpty()) return null;
        else {
        	return map.values().iterator().next().get(0).getRacine();
        }
    }

    /**
     * Rempli aléatoirement la base de données.
     * @throws SQLException
     * @throws FileNotFoundException
     * @throws IOException
     */
    public void populate(int nbRows) throws SQLException {
        boolean connected = this.is_connected();
        Random r = new Random();
        String value;
        String query = "";
        String subQuery;
        Integer size;
        SQLType type;
        String colName;
        String q1;
        String q2;
        Pattern p = Pattern.compile("^((TF)|(TO)).*");
        Matcher m;

        String fichier = "src/entrepot/dico.txt";

        if (!connected) {
            this.connect();
        }
        String[] types = {"TABLE"};
        ResultSet res = dbmd.getTables(connection.getCatalog(), "edd_database", "%", types);


        while (res.next()) {

            String tableName = res.getObject("TABLE_NAME").toString();
            m = p.matcher(tableName);
            if (m.matches()) {

                ArrayList<Triplet<String, Doublet<SQLType, Integer>, String>> fields = new ArrayList<Triplet<String, Doublet<SQLType, Integer>, String>>();

                ResultSet col = dbmd.getColumns(connection.getCatalog(), "edd_database", tableName, "%");


                while (col.next()) {
                    colName = col.getObject("COLUMN_NAME").toString();
                    type = SQLType.getSQLType((Integer) col.getObject("DATA_TYPE"));
                    size = Integer.decode(col.getObject("COLUMN_SIZE").toString());


                    fields.add(new Triplet<String, Doublet<SQLType, Integer>, String>(colName, new Doublet<SQLType, Integer>(type, size), null));
                }


                for (int i = 0; i < nbRows; i++) {
                    q1 = "";
                    q2 = "";
                    subQuery = "INSERT INTO " + tableName + " (";

                    for (Triplet<String, Doublet<SQLType, Integer>, String> elt : fields) {
                        type = elt.getSecond().getFirst();
                        size = elt.getSecond().getSecond();
                        colName = elt.getFirst();


                        if (!q1.isEmpty()) {
                            q1 = q1 + ", ";
                        }
                        q1 = q1 + "`" + colName + "`";


                        value = null;
                        if (type == SQLType.DATE) {
                            long val1 = new java.util.Date(0).getTime();
                            long val2 = new java.util.Date(System.currentTimeMillis()).getTime();

                            long randomTS = (long) (r.nextDouble() * (val2 - val1)) + val1;
                            java.sql.Date d = new java.sql.Date(randomTS);
                            value = d.toString();

                        } else if (type == SQLType.TIME) {
                            int h = r.nextInt(24);
                            int mn = r.nextInt(60);
                            int s = r.nextInt(60);
                            java.sql.Time t = new java.sql.Time((h * 3600 + mn * 60 + s) * 1000);
                            value = t.toString();

                        } else if (type == SQLType.TIMESTAMP) {
                            long val1 = new java.util.Date(0).getTime();
                            long val2 = new java.util.Date(System.currentTimeMillis()).getTime();

                            long randomTS = (long) (r.nextDouble() * (val2 - val1)) + val1;

                            java.sql.Timestamp ts = new java.sql.Timestamp(randomTS);
                            value = ts.toString();

                        } else if (type == SQLType.INTEGER || type == SQLType.SMALLINT || type == SQLType.BIGINT || type == SQLType.DECIMAL || type == SQLType.NUMERIC) {
                            value = Integer.toString(Math.abs(r.nextInt(((int) Math.pow(10, size)))));

                        } else if (type == SQLType.DOUBLE || type == SQLType.REAL) {
                            value = Float.toString(Math.abs(r.nextFloat()));

                        } else if (type == SQLType.VARCHAR || type == SQLType.CHAR || type == SQLType.LONGVARCHAR || type == SQLType.VARCHAR512) {
                            RandomAccessFile raf;
                            try {
                               try {
                                   raf = new RandomAccessFile(new File(fichier), "r");
                                    Integer n = r.nextInt(Integer.decode("" + raf.length()));
                                    raf.skipBytes(n);
                                    raf.readLine();
                                    value = raf.readLine();
                                } catch (NullPointerException npe) {
                                    raf = new RandomAccessFile(new File(fichier), "r");
                                    Integer n = r.nextInt(Integer.decode("" + raf.length()));
                                    raf.skipBytes(n/2);
                                    raf.readLine();
                                    value = raf.readLine();
                                }
                                try {
                                    value.substring(0, Math.min(value.length(), size));
                                } catch(NullPointerException npe) {
                                    value = "";
                                }
                                raf.close();
                            } catch (FileNotFoundException fnfe) {
                                Logger.getLogger(BDRelationelle.class.getName()).log(Level.SEVERE, null, fnfe);
                            } catch (IOException ioe) {
                                Logger.getLogger(BDRelationelle.class.getName()).log(Level.SEVERE, null, ioe);
                            }
                        }

                        if (!q2.isEmpty()) {
                            q2 = q2 + ", ";
                        }
                        q2 = q2 + "'" + value + "'";

                    }

                    subQuery = subQuery + q1 + ") VALUES (" + q2 + ");";

                    if (!query.isEmpty()) {
                        query = query + "___";
                    }
                    query = query + subQuery;
                    System.out.println(subQuery);
                }
            }
        }

        if (!query.isEmpty()) {
            this.executeUpdate(query);
        }

    }

    public void dropAllTable() throws SQLException {
        String[] types = {"TABLE"};
        ResultSet res = dbmd.getTables(connection.getCatalog(), "edd_database", "%", types);
        String query = "";

        while (res.next()) {
            String tableName = res.getObject("TABLE_NAME").toString();

            query += "DROP TABLE `" + tableName + "`;___";
        }

        executeUpdate(query);
    }
}
