<div class="messages messages--status">
<div class="messages__icon">
    <svg version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="24" height="24" viewBox="0 0 64 64">
        <path d="M54 8l-30 30-14-14-10 10 24 24 40-40z" fill="#000000">
        </path>
    </svg>
    <?php print $status; ?>
</div>
<?php if (!empty($pledge)) :?>
    <table>
        <tbody>
            <tr align="center">
                <td>Minimum Investment </td>
                <td>Suggested Investment</td>
                <td>Additional Increments</td>
                <td>Maximum Investment</td>
                <td>Exemption</td>
                </tr>
            <tr align="center">
                <td><?php print '$'. $pledge['minimum'] ?></td>
                <td><?php print '$'.$pledge['requested'] ?></td>
                <td><?php print '$'.$pledge['auto'] ?></td>
                <td><?php print $pledge['maximum'] ?></td>
                <td><?php print $pledge['rule'] ?></td>
            </tr>
        </tbody>
    </table>
<?php endif;?>
<hr>
<h3>New pledge of Individual Support</h3>
<?php print backdrop_render($pledge_form);?>
<div class="pledge-notes">
    <p><?php print base64_decode($pledge['notes']);?></p>
</div>