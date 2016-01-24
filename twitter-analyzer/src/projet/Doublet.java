package projet;

public class Doublet<X, Y> {

    private X first;
    private Y second;

    public Doublet(X x, Y y) {
        first = x;
        second = y;
    }

    public X getFirst() {
        return first;
    }

    public Y getSecond() {
        return second;
    }
}
