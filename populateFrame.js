

function populateFrame(iframe, url) {
    iframe.style.width = "100%";
    iframe.style.height = "10ex";
    alert("Loading iframe");
    iframe.src = url;

    ht =  jQuery($('#ifrm'), top.document).height();
    alert("jquery ht " + ht);
    //ibody = iframe.contentWindow.document.body;
    //ibody = iframe.contentWindow.document.documentElement;
    ibody = iframe.contentDocument; // .document.documentElement;
    alert("Document attribs are " + iframe.contentWindow.document.body.attributes);
    // ht = (ibody.scrollHeight +(ibody.offsetHeight - ibody.clientHeight)) + "px";
/*
    for (var key in ibody) {
	alert("Key " + key);
    }
*/
    // ht = ibody.scrollHeight + "px";
    alert("scroll ht " + ibody.scrollHeight);
    alert("offset ht " + ibody.offsetHeight);
    alert("Client ht " + ibody.clientHeight);
    alert("Ht " + ibody.height);

 
    // ht = iframe.contentWindow.document.body.scrollHeight + "px";
    iframe.style.height = ht;
   // document.getElementById('ifrm').src = parameter;
 // frames['iframe'].location.href = parameter;
}