#!/usr/bin/octave --persist


# retourne le vecteur caracteristique d'une image
function [rgb, nbrow, nbcol] = vect_car (url)
  rgbM = imread (strcat("../corpus/",url));
  [nbrow,nbcol,_] = size(rgbM);
  r = vec(rgbM(:,:,1));
  g = vec(rgbM(:,:,2));
  b = vec(rgbM(:,:,3));
  rgb = real([r g b]);
endfunction


# renvoie les vecteurs caracteristiques de l'image
# v => pixels visage
# nv => pixels non visage
function [v, nv] = definir_visage (url1, url2)
  [train,_,_] = vect_car (url1);
  [ref,_,_] = vect_car (url2);
  label = sum(ref,2) != 0;
  v = train(logical(label), :);
  nv = train(!logical(label), :);
endfunction


# retourne la classe la plus proche du vecteur de donnee
function [ c ] = retourne_classe ( data, mus, sigma_invs)
  [drow, dcol] = size(data);
  k = rows(mus);
  pas = 2^13-1;
  for m=1:pas:drow
    mf = min(m+pas, drow);
    datatmp = data(m:mf,:);
    drowtmp = size(datatmp,1);
    disttmp = [];
    for i=1:k
      dmu = datatmp .- repmat(mus(i,:),drowtmp,1);
      disttmp(:,i) = diag(dmu * sigma_invs(:,:,i) * dmu');
    endfor
    dist(m:mf,:) = disttmp;
  endfor
  [_,i] = sort (dist,2);
  c = i(:,1);
endfunction


# calcul les centriodes (mus) ainsi que les matrices de covariance inverse de chaque classe
function [mus, sigmas] = calcul_covariance (data, c, k)
  nbdata = size(data,2);
  for i=1:k
    tmp = data(c==i,:);
    if (rows(tmp)==0)
      mus(i,:) = rand (1,columns(data)) * 255;
      sigmas(:,:,i) = inv(cov (data));
    else
      covtmp = cov(tmp);
      [covm,rcond] = inv(covtmp);
      
      # ajoute une petite valeur a la diagonale
      # pour rentre la matrice inversible
      while (rcond < 1e-15)
	covtmp = covtmp .+ diag(ones(1, nbdata) * 0.001);
	[covm,rcond] = inv(covtmp);
      endwhile
      
      sigmas(:,:,i) = covm;
      mus(i,:) = mean(tmp);
      
    endif    
  endfor
endfunction


# algo kmean
# R => les k centroides
# C => les classes des donnees data
# covmatrix => les matrices de covariances inverses des classes
function [R, C, covmatrix] = kmean(data, k)
  [nbdata,dimdata] = size(data);
  [covinv, rcond] = inv(cov(data));
  deltat_eq = 1;
  for i=1:k
    covmatrix(:,:,i) = covinv;
  endfor
  mus = rand (k,dimdata) * 255;
  cpt = 1;
  
  stop = 0;
  while(!stop)
    printf("iteration = %d\n", cpt);
    cpt++;
    C = retourne_classe (data, mus, covmatrix);
    [R, covmatrix] = calcul_covariance(data, C, k);
    #if(all(R==mus))
    if(sum(sum(abs(R-mus))) <= deltat_eq)
      stop=1;
    else 
      mus = R;
    endif
  endwhile
endfunction


# retourne les classes des différents pixels passés en parametre
# en utilisant les donnees d'un classifieur
function [c] =  classer_image (img, rv, covv, rnv, covnv)
  [nbkv,nbdatav] = size (rv);
  [nbknv,nbdatanv] = size(rnv);
  r = [rv;rnv];
  co = covv;
  for i=1:size(rnv,1)
    co(:,:,i+size(rv,1)) = covnv(:,:,i);
  endfor
  
  
  c = retourne_classe(img, r, co);
endfunction


# retourne le taux d'erreur du classifieur
# !!!!!!!!!!!!!!!!!!! apprentissage doit avoir ete execute correctement avant
function [to] = calcul_erreur(img,ref,net)
  [rv,cv,covv,rnv,cnv,covnv] = loadData();
  
  [dimg,dx,dy] = vect_car(img);
  ccalcul = classer_image(dimg, rv, covv, rnv, covnv);
  ccalcul = ccalcul<=rows(rv);
  c2 = reshape(ccalcul,dx,dy);
  #c2 = netoyage4(ccalcul);
  ecrire_image (c2,net);

  ccalcul = vec(ccalcul);
  
  ccalculv = dimg(logical(ccalcul));
  ccalculnv = dimg(!logical(ccalcul));
  
  [cdonnev, cdonnenv] = definir_visage(img,ref);
  
  to = (abs(rows(ccalculv) - rows(cdonnev)) + abs(rows(ccalculnv) - rows(cdonnenv))) / rows(dimg);  
endfunction


# savegarde et charge les donnee du classifieur
# la sauvegarde est faite dans apprentissage()
function [] = saveData(rv,cv, covv, rnv, cnv, covnv)
  save -binary data.mat rv cv covv rnv cnv covnv;
endfunction

function [rv,cv,covv,rnv,cnv,covnv] = loadData()
  load ("data.mat", "rv", "cv", "covv", "rnv", "cnv", "covnv");
endfunction


# definit le classifieur sur le corpus d'apprentissage suivant
# photo 1
function [] = apprentissage()
  [v,nv] = definir_visage("Training_1.png", "ref1.png");
  
  k=5;
  
  [rv,cv,covv] = kmean(v,k);
  [rnv,cnv,covnv] = kmean(nv,k);
  
  saveData(rv,cv,covv,  rnv,cnv,covnv);
endfunction

# ecrit dans le fichier image.png l'image en noir et blanc des pixels de visage
function [] = ecrire_image(c,n)
  
  imwrite(c*255, n);
endfunction


# netoyage de l'image
function [img] = netoyage8(img)
  [nbrow,nbcol] = size(img);
  
  for i=1:nbrow-2
    for j=1:nbcol-2
      m = img(i:i+2, j:j+2);
      if(mean(mean(m)) > 0.5)
	img(i+1,j+1) = 1;
      else
	img(i+1,j+1) = 0;
      endif
    endfor
  endfor
endfunction

function [img] = netoyage4(img)
  [nbrow,nbcol] = size(img);

  for i=1:nbrow-2
    for j=1:nbcol-2
      m = img(i:i+2, j:j+2);
      m(1:2:3, 1:2:3) = zeros(2,2);
      if(mean(mean(m))+0.2222 > 0.5)
	img(i+1,j+1) = 1;
      else
	img(i+1,j+1) = 0;
      endif
    endfor
  endfor
endfunction



apprentissage();




#[rv,cv,covv,rnv,cnv,covnv] = loadData();


erreur = calcul_erreur("Training_1.png","ref1.png", "image.png");
printf("erreur apprentissage net 0 = %f\n", erreur*100);

erreur = calcul_erreur("Training_3.png","ref3.png", "imagetest.png");
printf("erreur test net 0 = %f\n", erreur*100);


#c = classer_image (img, rv, covv, rnv, covnv);

#c2 = netoyage(reshape(c<=rows(rv),nbrow, nbcol));

#ecrire_image (c2);


