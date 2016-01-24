package gui;

import java.awt.BorderLayout;
import java.awt.Dimension;
import javax.swing.JFrame;
import javax.swing.JPanel;
import entrepot.OlapArbre;

/**
 * JFrame pour cr�er les requetes
 * @author adel
 *
 */
public class QueryFrame extends JFrame {

    /**
     *
     */
    private static final long serialVersionUID = 1L;

    /**
     *
     * @param treeMod
     * 			racine de l'arbre olap
     */
    public QueryFrame(OlapArbre treeMod) {
        super();
        this.setVisible(true);
        this.setPreferredSize(new Dimension(800, 600));
        this.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        TreeSelectPanel tsp = new TreeSelectPanel(treeMod);
        QueryResultPanel qrp = new QueryResultPanel();
        QueryPanel qp = new QueryPanel(tsp, qrp);
        JPanel panelP = new JPanel();
        panelP.setLayout(new BorderLayout());
        panelP.add(tsp, BorderLayout.CENTER);
        panelP.add(qp, BorderLayout.EAST);
        panelP.add(qrp, BorderLayout.SOUTH);
        this.setContentPane(panelP);
        this.pack();

    }
}
