### Purpose of this tool
The tool fetches every email header from a gmail account.
Then it outputs a list of email addresses sorted by how many emails it found from that specific address.
With that information it is easy to recognize spamming addresses and delete them.

### Future improvements
Whenever I have the time, following features will be added:
- Automated authentication to gmail
- Automatic refreshing of the oauth2 access token
- Deleting emails belonging to an address from the tool itself
- Creating the config.txt and way to edit it via command line

### How to use it
1. Create new Google Cloud console project
2. In Google Cloud console, create new credentials with:
  - URL: http://127.0.0.1/
  - Name: whatever you like
  - You will need client id and client secret
3. Create new OAuth consent screen:
  - Test status
  - Add your email address to be analyzed as test user
4. Use the client id from step 1. credentials and go to this url:
  - This url will redirect you to the url that contains the code for getting the access token
```
https://accounts.google.com/o/oauth2/auth?client_id=<YOUR_CLIENT_ID>&redirect_uri=http://127.0.0.1&scope=https://mail.google.com/&email&response_type=code&include_granted_scopes=true&access_type=offline&state=state_parameter_passthrough_value
```
5. Copy the `code=<COPY THIS>` parameter value from the redirected url
6. Make a POST request with your client id and the code copied in the last step:
```
curl -X POST https://oauth2.googleapis.com/token \
-d "code=<PASTE CODE HERE>&client_id=<PASTE HERE>&client_secret=<PASTE HERE>&redirect_uri=http://127.0.0.1&access_type=offline&grant_type=authorization_code"
```
  - Response contains access token and refresh token
  - You can refresh the access token by sending a POST with the refresh token:
```
curl -X POST https://oauth2.googleapis.com/token \
-d "client_secret=<YOUR CLIENT SECRET>&grant_type=refresh_token&refresh_token=<YOUR REFRESH TOKEN>&client_id=<YOUR CLIENT ID>"
```
7. Create a config.txt in the executables directory in which:
  - Paste your email address on first row
  - Paste the refresh token on to the second row
  - Paste the access token on to the third row
  - Make sure the rows are seperated with newline since the file is read line by line
8. Start the tool with argument:
  - No arguments prints help screen
  - -u is used to fetch the email data
  - -a is used AFTER -u to analyze (and output) the email data
