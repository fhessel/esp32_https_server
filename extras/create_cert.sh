#!/bin/bash
set -e
#------------------------------------------------------------------------------
# cleanup any previously created files
rm -f exampleca.* example.* cert.h private_key.h

#------------------------------------------------------------------------------
# create a CA called "myca"

# create a private key
openssl genrsa -out exampleca.key 1024

# create certificate
cat > exampleca.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no
[ req_distinguished_name ]
C = DE
ST = BE
L = Berlin
O = MyCompany
CN = myca.local
EOF
openssl req -new -x509 -days 3650 -key exampleca.key -out exampleca.crt -config exampleca.conf
# create serial number file
echo "01" > exampleca.srl

#------------------------------------------------------------------------------
# create a certificate for the ESP (hostname: "myesp")

# create a private key
openssl genrsa -out example.key 1024
# create certificate signing request
cat > example.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no
[ req_distinguished_name ]
C = DE
ST = BE
L = Berlin
O = MyCompany
CN = esp32.local
EOF
openssl req -new -key example.key -out example.csr -config example.conf

# have myca sign the certificate
openssl x509 -days 3650 -CA exampleca.crt -CAkey exampleca.key -in example.csr -req -out example.crt

# verify
openssl verify -CAfile exampleca.crt example.crt

# convert private key and certificate into DER format
openssl rsa -in example.key -outform DER -out example.key.DER
openssl x509 -in example.crt -outform DER -out example.crt.DER

# create header files
echo "#ifndef CERT_H_" > ./cert.h
echo "#define CERT_H_" >> ./cert.h
xxd -i example.crt.DER >> ./cert.h
echo "#endif" >> ./cert.h

echo "#ifndef PRIVATE_KEY_H_" > ./private_key.h
echo "#define PRIVATE_KEY_H_" >> ./private_key.h
xxd -i example.key.DER >> ./private_key.h
echo "#endif" >> ./private_key.h

# Copy files to every example
for D in ../examples/*; do
  if [ -d "${D}" ] && [ -f "${D}/$(basename $D).ino" ]; then
    echo "Adding certificate to example $(basename $D)"
    cp ./cert.h ./private_key.h "${D}/"
  fi
done

echo ""
echo "Certificates created!"
echo "---------------------"
echo ""
echo "  Private key:      private_key.h"
echo "  Certificate data: cert.h"
echo ""
echo "Make sure to have both files available for inclusion when running the examples."
echo "The files have been copied to all example directories, so if you open an example"
echo " sketch, you should be fine."
