# onnx2c_ia_detect_objects
implement yolo standard to detect objects in a picture
#how to run ia_detect
first build and install opencv
first build and install protobuf ( useful to run onnx2c )
create alias in .bashrc

export LD_LIBRARY_PATH=/usr/local/lib
./ia_detect -s ./data/ -d ./results -i person.jpg

the processing image is generated into ./objets_detected
