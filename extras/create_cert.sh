#!/bin/bash
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
ST = HE
L = Darmstadt
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
ST = HE
L = Darmstadt
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
mkdir ../examples/cert
xxd -i example.crt.DER > ../examples/cert/cert.h
xxd -i example.key.DER > ../examples/cert/private_key.h
