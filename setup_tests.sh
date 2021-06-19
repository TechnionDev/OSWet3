mkdir -p "tests"
cd "tests"
curl https://pastebin.com/raw/We2xuzHu > server_tests.py
chmod +x server_tests.py
curl https://pastebin.com/raw/9vGvkU7i > pip_requirements.txt
pip3 install virtualenv
python3 -m venv venv
source venv/bin/activate
pip install -r pip_requirements.txt
echo ""
echo ""
echo "#####################################"
echo "You can run the tests now"
echo "From the 'tests' directory. Run 'python server_tests.py'"
echo "Make sure to change the path to your server binary if needed. It's one of the first lines in the python file"
