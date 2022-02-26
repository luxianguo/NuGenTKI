mkexe.sh anaGenerator -I${NUGENTKI}/include -lstyle -I${NUGENTKI}/style  -L${NUGENTKI}/style || exit

for TESTBEAM in list/Filelist_GEANT4_Piplus_Ar_1GeVc_1b.txt
                #list/Filelist_GEANT4_Pion_Ar_1GeV_v010000xstat.txt
                

    #list/Filelist*

#list/Filelist_GEANT4_Piplus_Ar_1GeVc_1b.txt list/Filelist_GEANT4_Electron_Ar1GeV_v010000xstat.txt list/Filelist_GEANT4_Electron_Ar500MeV.txt
do

ntostop=-999
#11900000
#10000000

for smearBit in 1 2 4 7
do
    echo ${smearBit}

tag=tmptmpTESTBEAM_smearBit${smearBit}_IsGoodFilter_HASthreshold_${ntostop}_WrestXrestQ2SPKAll_$(echo $TESTBEAM | awk -F\/ '{print $2}'| awk -F\. '{print $1}')

####################################

echo $tag

# TESTBEAM = 11
opt=${tag}TESTBEAM;          nohup ./doAna.sh $TESTBEAM         ${opt} 11 ${ntostop} ${smearBit} > see${opt}.log &

done
done

