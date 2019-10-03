echo "Building Gyach Enhanced"
cd ./gyach/
make
strip -s ./src/gyach
echo "Building Gyach Spanish locale support"
cd ./locale/es/
make
cd ../../
echo "Building PyTSP TrueSpeech library"
cd ../pytsp/
./Build
cp -f pytsp.py ../pyvoice/
cp -f _pytsp.so ../pyvoice/
cd ../pyvoice/locale/es/
echo "Building PyVoiceChat Spanish locale support"
make
echo "Building complete. If all went well, run 'make install'"