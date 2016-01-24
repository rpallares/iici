package gui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;

import entrepot.Contrainte;
import entrepot.OlapArbre;
import entrepot.OlapRequete;
import entrepot.OlapRequete.MalformedRequestException;
import entrepot.OlapRequete.Operator;

import net.miginfocom.swing.MigLayout;

/**
 * Jpanel pour creer les requetes et les executer sur le SGBD
 * 
 * @author adel
 * 
 */
public class QueryPanel extends JPanel {

	private static final long serialVersionUID = 2L;
	private JLabel lab1;
	private JLabel lab2;
	private JComboBox comboOpList;
	private QueryButtonListener listChemin;
	private QueryButtonListener listQuant;
	private ContraintePanel cp;
	private QueryResultPanel queryResultPanel;

	/**
	 * 
	 * @param tsp
	 *            Panel qui contient l'arbre olap
	 * @param qrp
	 *            panel qui affiche le resultat de la requete
	 */
	public QueryPanel(TreeSelectPanel tsp, QueryResultPanel qrp) {
		super();
		this.setLayout(new MigLayout());
		queryResultPanel = qrp;
		lab1 = new JLabel();
		lab1.setSize(10, 10);
		lab2 = new JLabel();
		// Operator[] opList =
		// Operator.getListeOperateur(listQuant.getSelected().get(0));
		Operator[] opList = Operator.getListeOperateur(null);
		comboOpList = new JComboBox(opList);

		JButton bt1 = new JButton("classificateur");
		listChemin = new QueryButtonListener(tsp, lab1, null,
				QueryButtonListener.MULTIPLE);
		listQuant = new QueryButtonListener(tsp, lab2, comboOpList,
				QueryButtonListener.UNIQUE);
		bt1.addActionListener(listChemin);
		JButton bt2 = new JButton("mesure");
		bt2.addActionListener(listQuant);

		JButton btNull = new JButton("Projection/o");
		btNull.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent arg0) {
				listChemin.setSelected(null);

			}

		});
		JButton queryButon = new JButton("gen query");
		queryButon.addActionListener(new ActionListener() {

			@Override
			public void actionPerformed(ActionEvent e) {
				OlapRequete query = createQuery(listChemin.getSelected(),
						listQuant.getSelected(),
						(Operator) comboOpList.getSelectedItem(),
						cp.getContrainte());
				String queryS = "";
				try {
					queryS = query.sqla(listQuant.getSelected().get(0)
							.getRacine());
					queryS += ";";
				} catch (MalformedRequestException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}

				System.out.println(queryS);
				if (MainFrame.ma_bd.is_connected()) {

					queryResultPanel.setQuery(MainFrame.ma_bd
							.executeQuery(queryS));

				}
				queryResultPanel.setQuery(lab1.getText(), lab2.getText(),
						(Operator) comboOpList.getSelectedItem());

			}
		});
		this.add(bt1, "cell 0 0");
		this.add(lab1, "cell 1 0");
		this.add(btNull, "cell 1 1");
		this.add(bt2, "cell 0 2");
		this.add(lab2, "cell 1 2");
		this.add(new JLabel("operation"), "cell 0 3");
		this.add(comboOpList, "cell 1 3");
		this.add(queryButon, "cell 1 4");
		cp = new ContraintePanel(tsp);
		this.add(cp, "cell 0 5");
	}

	/**
	 * methods qui cr�e une OlapRequete depuis les info dans le panel
	 * 
	 * @param chemins
	 *            liste des chemins selectionner
	 * @param quant
	 * 
	 * @param op
	 * @param contrainte
	 * @return
	 */
	public OlapRequete createQuery(ArrayList<OlapArbre> chemins,
			ArrayList<OlapArbre> quant, Operator op,
			ArrayList<Contrainte> contrainte) {
		// ArrayList<OlapArbre> chemins=listChemin.getSelected();
		// ArrayList<OlapArbre> quant=listQuant.getSelected();
		// Operator op=(Operator) comboOpList.getSelectedItem();
		ArrayList<Integer> cheminsInt = new ArrayList<Integer>();
		if (chemins != null) {

			for (OlapArbre ar : chemins) {
				cheminsInt.add(new Integer(ar.getId()));
			}
		}
		else{
			cheminsInt.add(new Integer(-1));
		}
		int q = quant.get(0).getId();

		return new OlapRequete(cheminsInt, new Integer(q), op, contrainte);
	}
}
