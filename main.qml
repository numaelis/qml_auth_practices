import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2
import QtQuick.Layouts 1.3
import QtWebEngine 1.8

Window {
    visible: true
    height: 300
    width: 200
    visibility:  Window.Maximized
    property bool granted: false

    Component.onCompleted: {
        AuthGateway.urlChanged.connect(funChanged);
        AuthGateway.statusChanged.connect(funStatusChanged);
        AuthGateway.notGranted.connect(funNotGranted);
        AuthGateway.granted.connect(funGranted);

        AuthGateway.loadCredentials({"web":{"client_id":"< CLIENT ID >",
                                            "project_id":"< project_id> ",
                                            "auth_uri":"https://accounts.google.com/o/oauth2/auth",
                                            "token_uri":"https://oauth2.googleapis.com/token",
                                            "auth_provider_x509_cert_url":"https://www.googleapis.com/oauth2/v1/certs",
                                            "client_secret":"< CLIENT SECRET >",
                                            "redirect_uris":["http://127.0.0.1:XXXX/"]}}
                                    );
        AuthGateway.setScope("https://www.googleapis.com/auth/calendar");

        dlogin.open();
        AuthGateway.initAuth();
        breconnect.enabled=false;

    }

    function funChanged(url){
        console.log("url",url);
        webev.url=url;
    }

    function funStatusChanged(status){
        console.log(status)
    }

    function funGranted(){
        granted=true;
        dlogin.close();
    }

    function funNotGranted(){
        granted=false;
        breconnect.enabled=true;
        dlogin.open();
    }

    Pane{
        anchors.fill: parent
        ColumnLayout{
            anchors.fill: parent
            RowLayout{
                Layout.alignment: Qt.AlignHCenter
                Label{
                    visible: granted
                    text:"Connected"
                }
            }
            //QAbstractOAuth::Status
            RowLayout{
                Button{
                    // visible: granted
                    text:"get calendar"
                    onClicked: {
                        var result = AuthGateway.getREST("https://www.googleapis.com/calendar/v3/users/me/calendarList");
                        console.log(JSON.stringify(result));
                        if(!result.hasOwnProperty("error")){
                            var items = result["items"];
                            caledarModel.clear();
                            for(var i=0, len = items.length;i<len;i++){
                                caledarModel.append({"summary":items[i].summary});
                            }
                        }
                    }
                }
                ListView{
                    Layout.preferredHeight: 300
                    Layout.preferredWidth: 200
                    clip: true
                    model: caledarModel
                    delegate: Text{
                        width: parent.width
                        text: summary
                    }
                }
            }

            RowLayout{
                Layout.fillHeight: true
            }
        }
    }

    ListModel{
        id:caledarModel
    }

    Dialog{
        id:dlogin
        width: 340
        height: 540
        anchors.centerIn: parent
        visible: !granted
        title: "login"
        modal: true
        closePolicy: Dialog.NoAutoClose
        contentItem: ColumnLayout{
            WebEngineView{
                id:webev
                visible: !granted
                Layout.preferredWidth: 300
                Layout.preferredHeight: 500
                Layout.alignment: Qt.AlignHCenter
                onUrlChanged:{
                    //console.log("www",url);
                }
                onTitleChanged: {
                    var strt = title.toString();
                    if(strt.indexOf("Error")>-1){
                        console.log("error connect...");
                    }
                }

            }
            Button{
                id:breconnect
                text: "reconnect"
                onClicked: {
                    AuthGateway.initAuth();
                }
            }
        }
    }
}
