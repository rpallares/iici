export PYTHONPATH="$PYTHONPATH:/net/public/tal/melt/lib/python2.6/site-packages:/net/public/tal/cjson/lib/python2.6/site-packages"
export MELT="/net/public/tal/melt"
export BONSAI="/net/public/tal/bonsai_v3.2"


$BONSAI/bin/bonsai_bky_parse_via_clust.sh corpus_correct.txt > arbres.txt
