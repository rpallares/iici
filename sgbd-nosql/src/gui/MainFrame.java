package gui;

import entrepot.BDRelationelle;
import entrepot.OlapArbre;
import entrepot.SQLType;
import entrepot.OlapArbre.NomExistantException;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JPanel;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.FileNotFoundException;
import java.sql.SQLException;

/**
 * creer la fenetre principale est initialise la connexion a la base de donn�es
 * @author adel
 *
 */
public class MainFrame extends JFrame {

    /**
     *
     */
    private static final long serialVersionUID = 1L;
    public static BDRelationelle ma_bd;

    public MainFrame() {
        super();
        this.setVisible(true);
        ma_bd = new BDRelationelle();

        ma_bd.connect();

        this.setPreferredSize(new Dimension(150, 150));
        this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        JPanel panelP = new JPanel(new BorderLayout());
        JButton btDemo = new JButton("demo Tree");
        JButton btDB = new JButton("Tree From DB");
        btDB.addActionListener(new ActionListener() {

            @Override
            public void actionPerformed(ActionEvent arg0) {
                if (ma_bd.is_connected()) {
                    try {
                        new QueryFrame(ma_bd.toOlapArbre());
                    } catch (SQLException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    } catch (NomExistantException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
                }

            }
        });
        JButton btnew = new JButton("new Tree");
        btnew.addActionListener(new ActionListener() {

            public void actionPerformed(ActionEvent e) {
                try {
                    MainFrame.ma_bd.dropAllTable();

                    new CreateTreeFrame();

                    MainFrame.ma_bd.populate(5);
                } catch (SQLException sqlEx) {
                    sqlEx.printStackTrace();
                }

            }
        });
        btDemo.addActionListener(new ActionListener() {

            @Override
            public void actionPerformed(ActionEvent e) {
                OlapArbre a;
                try {
                    a = new OlapArbre("O", SQLType.VARCHAR);

                    OlapArbre date = new OlapArbre("Date", SQLType.DATE,
                            new OlapArbre("Month", SQLType.INTEGER,
                            new OlapArbre("Year", SQLType.INTEGER)));
                    OlapArbre store = new OlapArbre("Store", SQLType.VARCHAR,
                            new OlapArbre("City", SQLType.VARCHAR,
                            new OlapArbre("Region", SQLType.VARCHAR)));
                    OlapArbre product = new OlapArbre("Product",
                            SQLType.VARCHAR);
                    product.ajouterFils(new OlapArbre("Category",
                            SQLType.VARCHAR));
                    product.ajouterFils(new OlapArbre("Supplier",
                            SQLType.VARCHAR));

                    a.ajouterFils(date);
                    a.ajouterFils(store);
                    a.ajouterFils(product);

                    a.ajouterFils(new OlapArbre("Quantity", SQLType.INTEGER));
                    if (MainFrame.ma_bd.is_connected()) {
                        MainFrame.ma_bd.dropAllTable();
                        MainFrame.ma_bd.executeUpdate(a.toSQLStarSchema());
                        MainFrame.ma_bd.populate(500);
                    }
                    new QueryFrame(a);
                } catch (NomExistantException e1) {
                    e1.printStackTrace();

                } catch (SQLException sqlEx) {
                    sqlEx.printStackTrace();

                }

            }
        });
        panelP.add(btnew, BorderLayout.NORTH);
        panelP.add(btDB, BorderLayout.CENTER);
        panelP.add(btDemo, BorderLayout.SOUTH);
        this.setContentPane(panelP);
        this.pack();

    }

    public static void main(String[] args) throws NomExistantException {

        new MainFrame();

    }
}
